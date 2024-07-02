<?php
/** @noinspection HtmlDeprecatedTag */

namespace hng2_modules\autopush;

use hng2_base\account;
use hng2_base\module;
use hng2_media\media_repository;
use hng2_modules\posts\post_record;
use Abraham\TwitterOAuth\TwitterOAuth;
use hng2_modules\posts\posts_repository;
use phpQuery;

class toolbox
{
    public function __construct()
    {
        global $modules;
        
        /** @var module $current_module */
        $current_module = $modules["autopush"];
        
        require_once $current_module->abspath . "/lib/twitteroauth/autoload.php";
    }
    
    /**
     * @param string             $network
     * @param string             $method          as_link|as_pieces
     * @param array              $endpoint_data
     * @param post_record|string $pushing_element Post record or ULR being pushed or "message:override"
     * @param account            $sender
     * @param bool               $notify_errors   If true, errors will be notified to the sender.
     *                                            If false, errors will be logged on $config->globals["@autopush:sending_errors"]
     * @param string             $as_link_message Message to send with the link (if the method is "as_link".
     */
    public function push($network, $method, $endpoint_data, $pushing_element, $sender, $notify_errors = true, $as_link_message = "")
    {
        global $modules, $config;
        
        $current_module = $modules["autopush"];
        
        # These are overriden by the private method
        $config->globals["@autopush:sending_errors"] = array();
        $config->globals["@autopush:messages_count"] = 0;
        $config->globals["@autopush:messages_sent"]  = 0;
        
        $notifications_target = $notify_errors ? $sender->id_account : false;
        
        switch( $network )
        {
            case "twitter":
                $this->post_to_twitter($method, $endpoint_data, $pushing_element, $notifications_target, $as_link_message);
                break;
            case "discord":
                $this->post_to_discord($method, $endpoint_data, $pushing_element, $notifications_target, $as_link_message);
                break;
            case "telegram":
                $this->post_to_telegram($method, $endpoint_data, $pushing_element, $notifications_target, $as_link_message);
                break;
            case "linkedin_profile":
                $this->post_to_linkedin("profile", $method, $endpoint_data, $pushing_element, $notifications_target, $as_link_message);
                break;
            case "linkedin_company":
                $this->post_to_linkedin("company", $method, $endpoint_data, $pushing_element, $notifications_target, $as_link_message);
                break;
        }
        
        if( $config->globals["@autopush:messages_sent"] == 0 ) return;
        
        $message = unindent(replace_escaped_objects(
            $current_module->language->messages->pushed_info, array(
                '{$pushed_as}' => $current_module->language->pushing_methods->{$method},
                '{$sender}'    => $sender->get_processed_display_name(),
                '{$date}'      => date("Y-m-d H:i:s"),
                '{$sent}'      => $config->globals["@autopush:messages_sent"],
                '{$count}'     => $config->globals["@autopush:messages_count"],
            )
        ));
        
        if( is_object($pushing_element) )
            $pushing_element->set_meta("@autopush:$network.last_push", $message);
    }
    
    /**
     * @param string             $method               as_link | as_pieces
     * @param array              $endpoint_data        = [
     *                                                       "title" => "string",
     *                                                       "api_key" => "string",
     *                                                       "api_secret" => "string",
     *                                                       "token_key" => "string",
     *                                                       "token_secret" => "string
     *                                                   ]
     * @param post_record|string $pushing_element      Post record or URL being pushed or "message:override"
     * @param int|bool           $notifications_target Account id to notify or false for no notifications
     * @param string             $as_link_message      Message to send with the link (if the method is "as_link".
     */
    private function post_to_twitter($method, $endpoint_data, $pushing_element, $notifications_target, $as_link_message)
    {
        global $modules, $config;
        $current_module = $modules["autopush"];
        
        if( is_string($pushing_element) ) $content = $pushing_element;
        else                              $content = $this->get_processed_content($pushing_element, $method);
        
        if( empty($content) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->empty_content, "Twitter", $endpoint_data["title"]
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        $connection = new TwitterOAuth(
            $endpoint_data["api_key"],
            $endpoint_data["api_secret"],
            $endpoint_data["token_key"],
            $endpoint_data["token_secret"]
        );
        $connection->setTimeouts(10, 15);
        
        if( $method == "as_link" )
        {
            if( is_object($pushing_element) )
                $title = empty($as_link_message) ? $pushing_element->get_processed_excerpt(true) : $as_link_message;
            else
                $title = $as_link_message;
            
            $type  = $content == "message:override"
                   ? trim($current_module->language->post_types->message)
                   : trim($current_module->language->post_types->link);
            if( strlen($title) > 200 ) $title = make_excerpt_of($title, 200, false);
            $link  = $content == "message:override" ? "" : $content;
            
            $data = array(
                "text" => trim("$title $link")
            );
            
            $res = $this->tweet(
                $connection, "tweets", $data, $type, $title, $endpoint_data, $notifications_target
            );
            
            if( $res ) $config->globals["@autopush:messages_sent"]++;
            
            return;
        }
        
        #
        # Paragraphs
        #
        
        $previous_tweet_id   = 0;
        $tweet_author_handle = "";
        foreach($content as $message)
        {
            if( substr($message, 0, 7) == "<image>" )
            {
                $type  = trim($current_module->language->post_types->image);
                $url   = str_replace("<image>", "", $message);
                
                $delete_file_after_upload = false;
                if( stristr($url, "/mediaserver/") !== false )
                {
                    $file  = $config->datafiles_location . "/uploaded_media/"
                           . preg_replace("#http.*/mediaserver/#i", "", $url);
                    $title = ucwords(str_replace(array("-", "_", "."), " ", basename($file)));
                }
                else
                {
                    $ch = curl_init();
                    curl_setopt($ch, CURLOPT_URL,            $url);
                    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
                    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 10);
                    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
                    
                    $data = curl_exec($ch);
                    
                    if( curl_error($ch) )
                    {
                        $error = unindent(sprintf(
                            $current_module->language->messages->cannot_fetch_image,
                            $url, curl_error($ch)
                        ));
                        
                        $config->globals["@autopush:sending_errors"][] = $error;
                        
                        if( $notifications_target ) send_notification($notifications_target, "error", $error);
                        
                        curl_close($ch);
                        continue;
                    }
                    
                    $info = curl_getinfo($ch, CURLINFO_CONTENT_TYPE);
                    
                    if( ! preg_match("#image/(png|jpg|jpeg|gif)#i", $info) )
                    {
                        $error = unindent(sprintf(
                            $current_module->language->messages->invalid_image_type,
                            $url, curl_error($ch)
                        ));
                        
                        $config->globals["@autopush:sending_errors"][] = $error;
                        
                        if( $notifications_target ) send_notification($notifications_target, "error", $error);
                        
                        curl_close($ch);
                        continue;
                    }
                    
                    if( empty($data) )
                    {
                        $error = trim($current_module->language->messages->image_is_empty);
                        
                        $config->globals["@autopush:sending_errors"][] = $error;
                        
                        if( $notifications_target ) send_notification($notifications_target, "error", $error);
                        
                        curl_close($ch);
                        continue;
                    }
                    
                    $ext  = strtolower(end(explode("/", $info)));
                    $file = $config->datafiles_location . "/tmp/img-" . microtime(true) . "." . $ext;
                    file_put_contents($file, $data);
                    $delete_file_after_upload = true;
                    curl_close($ch);
                    $title = $url;
                }
                
                $res = $this->tweet(
                    $connection, "media/upload", array("media" => $file), $type, $title, $endpoint_data, $notifications_target
                );
                
                if( ! $res ) continue;
                
                if( $delete_file_after_upload ) @unlink($file);
                
                $media_id = $res->media_id_string;
                
                $data = array("text" => $title, "media_ids" => $media_id);
                if( ! empty($previous_tweet_id) )
                {
                    $data["reply"]["in_reply_to_tweet_id"] = $previous_tweet_id;
                    $data["text"] = "@{$tweet_author_handle} {$data["text"]}";
                }
                
                $res = $this->tweet(
                    $connection, "tweets", $data, $type, $title, $endpoint_data, $notifications_target
                );
                
                if( $res )
                {
                    $previous_tweet_id   = $res->id;
                    $tweet_author_handle = $res->user->screen_name;
                    $config->globals["@autopush:messages_sent"]++;
                }
            }
            elseif( substr($message, 0, 7) == "<video>" ) # As link
            {
                $type    = trim($current_module->language->post_types->video);
                $message = str_replace("<video>", "", $message);
                $title   = basename($message);
                
                $data = array("text" => $message);
                if( ! empty($previous_tweet_id) )
                {
                    $data["reply"]["in_reply_to_tweet_id"] = $previous_tweet_id;
                    $data["text"] = "@{$tweet_author_handle} {$data["text"]}";
                }
                
                $res = $this->tweet(
                    $connection, "tweet", $data, $type, $title, $endpoint_data, $notifications_target
                );
                
                if( $res )
                {
                    $previous_tweet_id   = $res->id;
                    $tweet_author_handle = $res->user->screen_name;
                    $config->globals["@autopush:messages_sent"]++;
                }
            }
            else
            {
                $type  = trim($current_module->language->post_types->message);
                $title = make_excerpt_of($message);
                
                $data = array("text" => make_excerpt_of($message, 200));
                if( ! empty($previous_tweet_id) )
                {
                    $data["reply"]["in_reply_to_tweet_id"] = $previous_tweet_id;
                    $data["text"] = "@{$tweet_author_handle} {$data["text"]}";
                }
                
                $res = $this->tweet(
                    $connection, "tweet", $data, $type, $title, $endpoint_data, $notifications_target
                );
                
                if( $res )
                {
                    $previous_tweet_id   = $res->id;
                    $tweet_author_handle = $res->user->screen_name;
                    $config->globals["@autopush:messages_sent"]++;
                }
            }
        }
    }
    
    /**
     * @param TwitterOAuth $connection
     * @param string       $twitter_method
     * @param array        $data
     * @param string       $type
     * @param string       $title
     * @param array        $endpoint_data        = [
     *                                                 "title" => "string",
     *                                                 "consumer_key" => "string",
     *                                                 "consumer_secret" => "string",
     *                                                 "token" => "string",
     *                                                 "token_secret" => "string"
     *                                             ]
     * @param int|bool     $notifications_target Account id to notify or false for no notifications
     * 
     * @return object|bool
     */
    private function tweet($connection, $twitter_method, $data, $type, $title, $endpoint_data, $notifications_target)
    {
        global $modules, $config;
        $current_module = $modules["autopush"];
        
        if( $twitter_method == "media/upload" )
            $res = $connection->upload($twitter_method, $data);
        else
            $res = $connection->post($twitter_method, $data, true);
        sleep( 1 );
        
        if( empty($res) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->empty_twitter_res,
                $type, $title
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return false;
        }
        
        if( ! is_object($res) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->unknown_twitter_res,
                $type, $title, print_r($res, true)
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return false;
        }
        
        if( empty($res->data) )
        {
            $code    = $res->status;
            $message = "$res->title: $res->detail";
            
            $error = unindent(sprintf(
                $current_module->language->messages->cannot_post_to_twitter,
                $type, $title, $endpoint_data["title"], "[{$code}] {$message}"
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return false;
        }
        
        return $res;
    }
    
    /**
     * @param string      $method               as_link|as_pieces
     * @param array       $endpoint_data        = ["title" => "string", "webhook" => "url"]
     * @param post_record $pushing_element      Post record or URL to post or "message:override"
     * @param int|bool    $notifications_target Account id to notify or false for no notifications
     * @param string      $as_link_message      Message to send with the link (if the method is "as_link".
     */
    private function post_to_discord($method, $endpoint_data, $pushing_element, $notifications_target, $as_link_message)
    {
        global $modules, $config;
        
        $current_module = $modules["autopush"];
        
        $url = $endpoint_data["webhook"] . "?wait=true";
        $ch  = curl_init();
        
        curl_setopt($ch, CURLOPT_URL,            $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 10);
        curl_setopt($ch, CURLOPT_POST,           1);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
        
        if( is_string($pushing_element) ) $content = $pushing_element;
        else                              $content = $this->get_processed_content($pushing_element, $method);
        
        if( empty($content) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->empty_content, "Discord", $endpoint_data["title"]
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        $payloads = array();
        
        if( $method == "as_link" )
        {
            if( is_object($pushing_element) )
                $title = empty($as_link_message) ? $pushing_element->get_processed_excerpt(true) : $as_link_message;
            else
                $title = $as_link_message;
            
            $link = $content == "message:override" ? "" : $content;
            $type = $content == "message:override"
                  ? trim($current_module->language->post_types->message)
                  : trim($current_module->language->post_types->link);
            
            $payloads[] = (object) array(
                "type"  => $type,
                "title" => make_excerpt_of($link),
                "data"  => array(
                    "content" => trim($title . "\n" . $link)
                )
            );
        }
        else
        {
            foreach($content as $message)
            {
                if( substr($message, 0, 7) == "<image>" )
                {
                    $message    = str_replace("<image>", "", $message);
                    $payloads[] = (object) array(
                        "type"  => trim($current_module->language->post_types->image),
                        "title" => basename($message),
                        "data"  => array(
                            "embeds" => array(
                                (object) array(
                                    "image" => (object) array(
                                        "url"   => $message
                                    )
                                )
                            )
                        )
                    );
                }
                elseif( substr($message, 0, 7) == "<video>" )
                {
                    $message    = str_replace("<video>", "", $message);
                    $payloads[] = (object) array(
                        "type"  => trim($current_module->language->post_types->video),
                        "title" => basename($message),
                        "data"  => array(
                            "embeds" => array(
                                (object) array(
                                    "title" => basename($message),
                                    "url"   => $message
                                )
                            )
                        )
                    );
                }
                else
                {
                    $payloads[] = (object) array(
                        "type"  => trim($current_module->language->post_types->link),
                        "title" => make_excerpt_of($message),
                        "data"  => (object) array(
                            "content" => $message
                        )
                    );
                }
            }
        }
        
        foreach($payloads as $item)
        {
            $item_type   = $item->type;
            $item_title  = $item->title;
            $payload     = $item->data;
            $post_string = json_encode($payload);
            
            curl_setopt($ch, CURLOPT_HTTPHEADER, array(
                "Content-Type: application/json; charset=UTF-8",
                "Content-Length: " . strlen($post_string)
            ));
            curl_setopt($ch, CURLOPT_POSTFIELDS, $post_string);
            $res = curl_exec($ch);
            sleep(1);
            
            if( curl_error($ch) )
            {
                $error = unindent(sprintf(
                    $current_module->language->messages->cannot_post_to_discord,
                    $item_type, $item_title, curl_error($ch)
                ));
                
                $config->globals["@autopush:sending_errors"][] = $error;
                
                if( $notifications_target ) send_notification($notifications_target, "error", $error);
                
                continue;
            }
            
            if( empty($res) )
            {
                $error = unindent(sprintf(
                    $current_module->language->messages->empty_discord_res,
                    $item_type, $item_title
                ));
                
                $config->globals["@autopush:sending_errors"][] = $error;
                
                if( $notifications_target ) send_notification($notifications_target, "error", $error);
                
                continue;
            }
            
            $obj = json_decode($res);
            if( ! is_object($obj) )
            {
                $error = unindent(sprintf(
                    $current_module->language->messages->unknown_discord_res,
                    $item_type, $item_title, print_r($res, true)
                ));
                
                $config->globals["@autopush:sending_errors"][] = $error;
                
                if( $notifications_target ) send_notification($notifications_target, "error", $error);
                
                continue;
            }
            
            $config->globals["@autopush:messages_sent"]++;
        }
    }
    
    /**
     * @param string      $method               as_link|as_pieces
     * @param array       $endpoint_data        = ["title" => "string", "token" => "string", "target" => "string"]
     * @param post_record $pushing_element      Post record or URL to post or "message:override"
     * @param int|bool    $notifications_target Account id to notify or false for no notifications
     * @param string      $as_link_message      Message to send with the link (if the method is "as_link".
     */
    private function post_to_telegram($method, $endpoint_data, $pushing_element, $notifications_target, $as_link_message)
    {
        global $modules, $config;
        
        $current_module = $modules["autopush"];
        
        if( is_string($pushing_element) ) $content = $pushing_element;
        else                              $content = $this->get_processed_content($pushing_element, $method);
        
        if( empty($content) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->empty_content, "Telegram", $endpoint_data["title"]
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        $payloads = array();
        
        list($chat_id, $message_thread_id) = explode("#", $endpoint_data["target"]);
        
        if( $method == "as_link" )
        {
            if( is_object($pushing_element) )
                $title = empty($as_link_message) ? $pushing_element->get_processed_excerpt(true) : $as_link_message;
            else
                $title = $as_link_message;
            
            $caption = str_replace(array("http://", "https://", "www."), "", make_excerpt_of($content, 50));
            $s_text  = $content == "message:override"
                     ? $as_link_message
                     : sprintf('%s <a href="%s">%s</a>', $title, $content, $caption);
            
            $payloads[] = (object) array(
                "endpoint" => "sendMessage",
                "title"    => $title,
                "data" => array(
                    "chat_id"    => $chat_id,
                    "message_thread_id" => $message_thread_id,
                    "text"       => $s_text,
                    "parse_mode" => "HTML",
                )
            );
        }
        else
        {
            foreach($content as $message)
            {
                if( substr($message, 0, 7) == "<image>" )
                {
                    $message    = str_replace("<image>", "", $message);
                    $payloads[] = (object) array(
                        "endpoint" => "sendPhoto",
                        "title"    => $message,
                        "data" => array(
                            "chat_id"    => $chat_id,
                            "message_thread_id" => $message_thread_id,
                            "photo"      => $message,
                            "caption"    => basename($message),
                            "parse_mode" => "HTML",
                        )
                    );
                }
                elseif( substr($message, 0, 7) == "<video>" )
                {
                    $message    = str_replace("<video>", "", $message);
                    $payloads[] = (object) array(
                        "endpoint" => "sendVideo",
                        "title"    => $message,
                        "data" => array(
                            "chat_id"    => $chat_id,
                            "message_thread_id" => $message_thread_id,
                            "video"      => $message,
                            "caption"    => basename($message),
                            "parse_mode" => "HTML",
                        )
                    );
                }
                else
                {
                    $payloads[] = (object) array(
                        "endpoint" => "sendMessage",
                        "title"    => make_excerpt_of($message),
                        "data" => array(
                            "chat_id"    => $chat_id,
                            "message_thread_id" => $message_thread_id,
                            "text"       => make_excerpt_of($message, 3000),
                            "parse_mode" => "HTML",
                        )
                    );
                }
            }
        }
        
        foreach($payloads as $item)
        {
            $url = "https://api.telegram.org/bot{$endpoint_data["token"]}/{$item->endpoint}";
            $ch  = curl_init();
            curl_setopt($ch, CURLOPT_URL,            $url);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
            curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 10);
            curl_setopt($ch, CURLOPT_POST,           1);
            curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
            
            $item_type  = $item->endpoint;
            $item_title = $item->title;
            $payload    = $item->data;
            
            curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($payload));
            $res = curl_exec($ch);
            sleep(1);
            
            if( curl_error($ch) )
            {
                $error = unindent(sprintf(
                    $current_module->language->messages->cannot_post_to_telegram,
                    $item_type, $item_title, curl_error($ch)
                ));
                
                $config->globals["@autopush:sending_errors"][] = $error;
                
                if( $notifications_target ) send_notification($notifications_target, "error", $error);
                
                curl_close($ch);
                
                continue;
            }
            
            curl_close($ch);
            
            if( empty($res) )
            {
                $error = unindent(sprintf(
                    $current_module->language->messages->empty_telegram_res,
                    $item_type, $item_title
                ));
                
                $config->globals["@autopush:sending_errors"][] = $error;
                
                if( $notifications_target ) send_notification($notifications_target, "error", $error);
                
                continue;
            }
            
            $obj = json_decode($res);
            if( ! is_object($obj) )
            {
                $error = unindent(sprintf(
                    $current_module->language->messages->unknown_telegram_res,
                    $item_type, $item_title, print_r($res, true)
                ));
                
                $config->globals["@autopush:sending_errors"][] = $error;
                
                if( $notifications_target ) send_notification($notifications_target, "error", $error);
                
                continue;
            }
            
            if( ! $obj->ok )
            {
                $error = unindent(sprintf(
                    $current_module->language->messages->telegram_api_error_received,
                    $item_type, $item_title, $obj->description
                ));
                
                $config->globals["@autopush:sending_errors"][] = $error;
                
                if( $notifications_target ) send_notification($notifications_target, "error", $error);
                
                continue;
            }
            
            $config->globals["@autopush:messages_sent"]++;
        }
    }
    
    /**
     * @param string      $endpoint_type        profile | company
     * @param string      $method               as_link is the only method accepted
     * @param array       $endpoint_data        = ["title" => "string", "token" => "string", "target" => "string"]
     * @param post_record $pushing_element      Post record or URL to post or "message:override"
     * @param int|bool    $notifications_target Account id to notify or false for no notifications
     * @param string      $as_link_message      Message to send with the link (if the method is "as_link".
     */
    private function post_to_linkedin($endpoint_type, $method, $endpoint_data, $pushing_element, $notifications_target, $as_link_message)
    {
        global $modules, $config, $settings;
        
        $current_module = $modules["autopush"];
    
        if( ! is_object($pushing_element) )
            $title = $as_link_message;
        else
            $title = empty($as_link_message) ? $pushing_element->get_processed_excerpt(true) : $as_link_message;
        
        if( is_string($pushing_element) && $pushing_element == "message:override" )     $item_type = "message";
        elseif( is_string($pushing_element) && $pushing_element != "message:override" ) $item_type = "link";
        else                                                                            $item_type = "post";
        
        $encryption_key  = "bfxFUbgQbQ7QNCmVaK";
        $decrypted_token = decrypt($endpoint_data["token"], $encryption_key);
        list($token_string, $token_expiration) = explode("\t", $decrypted_token);
        
        if( ! is_numeric($token_expiration) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->malformed_linkedin_token,
                $item_type, $title, $endpoint_data["title"]
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        $now  = date("Y-m-d H:i:s");
        $then = date("Y-m-d H:i:s", $token_expiration);
        
        if( $now > $then )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->linkedin_token_expired,
                $item_type, $title, $endpoint_data["title"]
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        $decrypted_res   = decrypt($endpoint_data["target"], $encryption_key);
        $resource_parts  = explode("\t", $decrypted_res);
        
        if( count($resource_parts) < 1 || count($resource_parts) > 2 )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->malformed_linkedin_resource_id,
                $item_type, $title, $endpoint_data["title"]
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        if( ! preg_match('/^user:(.+),(.+)$/', $resource_parts[0], $matches) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->invalid_linkedin_profile_id,
                $item_type, $title, $endpoint_data["title"]
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        $res_user_id   = $matches[1];
        $res_user_name = $matches[2];
        
        $res_comp_id   = "";
        $res_comp_name = "";
        if( $endpoint_type == "company" )
        {
            if( ! preg_match('/^company:(.+),(.+)$/', $resource_parts[1], $matches) )
            {
                $error = unindent(sprintf(
                    $current_module->language->messages->invalid_linkedin_company_id,
                    $item_type, $title, $endpoint_data["title"], $res_user_name
                ));
                
                $config->globals["@autopush:sending_errors"][] = $error;
                
                if( $notifications_target ) send_notification($notifications_target, "error", $error);
                
                return;
            }
            
            $res_comp_id   = $matches[1];
            $res_comp_name = $matches[2];
        }
        
        $resource_urn  = $endpoint_type == "profile" ? "urn:li:person:$res_user_id" : "urn:li:organization:$res_comp_id";
        $resource_name = $endpoint_type == "profile" ? $res_user_name : "$res_user_name » $res_comp_name";
        
        if( $method != "as_link" )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->as_link_only, "{$endpoint_data['title']} ($resource_name)"
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        $item = (object) array(
            "type"     => $item_type,
            "endpoint" => "v2/shares",
            "title"    => $title,
            "data"     => null,
            "headers"  => array(
                "Authorization: Bearer {$token_string}", 
                "Content-type: application/json", 
                "x-li-format: json"
            )
        );
        
        if( is_string($pushing_element) && $pushing_element == "message:override" )
        {
            #
            # We're posting a message. It comes in $as_link_message
            #
            $item->data = (object) array(
                "owner" => $resource_urn,
                "text" => (object) array(
                    "text" => $as_link_message
                ),
                "distribution" => (object) array(
                    "linkedInDistributionTarget" => (object) array()
                )
            );
        }
        elseif( is_string($pushing_element) && $pushing_element != "message:override" )
        {
            #
            # We're posting a link.
            #
            
            $post = $this->get_post_from_permalink($pushing_element);
            
            $item->data = (object) array(
                "owner" => $resource_urn,
                "content" => (object) array(
                    "contentEntities" => array(
                        (object) array(
                            "entityLocation" => $pushing_element,
                        )
                    ),
                    "title" => is_object($post) ? $post->title : $settings->get("engine.website_name"),
                ),
                "text" => (object) array(
                    "text" => $as_link_message
                ),
                "distribution" => (object) array(
                    "linkedInDistributionTarget" => (object) array()
                )
            );
        }
        else
        {
            #
            # We're posting a post
            #
            $item->data = (object) array(
                "owner" => $resource_urn,
                "content" => (object) array(
                    "contentEntities" => array(
                        (object) array(
                            "entityLocation" => $pushing_element->get_permalink(true),
                            "thumbnails" => array(
                                (object) array(
                                    "resolvedUrl" => "{$config->full_root_url}/{$pushing_element->featured_image_thumbnail}"
                                )
                            )
                        )
                    ),
                    "title" => $pushing_element->title
                ),
                "subject" => $pushing_element->title,
                "text" => (object) array(
                    "text" => $as_link_message
                ),
                "distribution" => (object) array(
                    "linkedInDistributionTarget" => (object) array()
                )
            );
        }
        
        $url = "https://api.linkedin.com/{$item->endpoint}";
        $ch  = curl_init();
        curl_setopt($ch, CURLOPT_URL,            $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 10);
        curl_setopt($ch, CURLOPT_POST,           1);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
        curl_setopt($ch, CURLOPT_HTTPHEADER,     $item->headers);
        curl_setopt($ch, CURLOPT_POSTFIELDS,     json_encode($item->data));
        
        $item_type  = $item->type;
        $item_title = $item->title;
        
        // $logentry = sprintf(
        //     "----------------------------------------------------------------\n" .
        //     "[%s] Posting to %s\n" .
        //     "----------------------------------------------------------------\n" .
        //     "Details:\n• Headers: %s\n• Payload: %s\n• Endpoint data: %s\n• Token: %s\n",
        //     date("Y-m-d H:i:s"),
        //     $url,
        //     trim(print_r($item->headers, true)),
        //     trim(print_r($item->data, true)),
        //     trim(print_r($endpoint_data, true)),
        //     $token_string
        // );
        // @file_put_contents("{$config->logfiles_location}/autopush_for_linkedin_debug.log", $logentry, FILE_APPEND);
        $res = curl_exec($ch);
        // $logentry = "• Curl error: " . curl_error($ch) . "\n"
        //           . "• Response: $res\n\n";
        // @file_put_contents("{$config->logfiles_location}/autopush_for_linkedin_debug.log", $logentry, FILE_APPEND);
        sleep(1);
        
        if( curl_error($ch) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->cannot_post_to_linkedin,
                $item_type, $item_title, $resource_name, curl_error($ch)
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            curl_close($ch);
            
            return;
        }
        
        curl_close($ch);
        
        if( empty($res) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->empty_linkedin_res,
                $resource_name, $item_type, $item_title
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        $obj = @json_decode($res);
        if( ! is_object($obj) )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->unknown_linkedin_res,
                $item_type, $item_title, $resource_name, strip_tags($res)
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        if( $obj->serviceErrorCode )
        {
            $error = unindent(sprintf(
                $current_module->language->messages->linkedin_api_error_received,
                $item_type, $item_title, $resource_name, $obj->message
            ));
            
            $config->globals["@autopush:sending_errors"][] = $error;
            
            if( $notifications_target ) send_notification($notifications_target, "error", $error);
            
            return;
        }
        
        $config->globals["@autopush:messages_sent"]++;
    }
    
    /**
     * @param post_record $post
     * @param string      $method as_link|as_pieces
     * 
     * @return string|array
     */
    private function get_processed_content($post, $method)
    {
        global $config;
        
        $config->globals["@autopush:messages_count"] = 1;
        
        if( $method == "as_link" ) return $post->get_permalink(true);
        
        if( ! class_exists('phpQuery') ) include_once ROOTPATH . "/lib/phpQuery-onefile.php";
        
        $config->globals["@autopush:paracollection"] = array();
        
        $ct = externalize_urls($post->get_processed_content());
        $ct = str_replace("<hr>",          "", $ct);
        $ct = str_replace("<hr />",        "", $ct);
        $ct = str_replace("<p>&nbsp;</p>", "", $ct);
        $ct = str_replace("<p> </p>",      "", $ct);
        $ct = str_replace("<p></p>",       "", $ct);
        $pq = phpQuery::newDocumentHTML( $ct );
        
        $pq->find('div.video_container')->each(function($e) {
            global $config;
            static $media_repository = null;
            if( is_null($media_repository) ) $media_repository = new media_repository();
            $_this   = pq($e);
            $item_id = $_this->find("video")->attr("data-id-media");
            $item    = $media_repository->get($item_id);
            if( is_null($item) )
            {
                $_this->remove();
            }
            else
            {
                $_this->replaceWith("<p><video><source src='{$config->full_root_url}/data/uploaded_media/{$item->path}'></video></p>");
            }
        });
        
        $pq->find("h1, h2, h3, h4, h5, h6")->each(function($e) {
            global $config;
            $contents = trim(pq($e)->text());
            if( ! empty($contents) )
                $config->globals["@autopush:paracollection"][] = $contents;
        });
        
        $pq->find("p")->each(function($e) {
            global $config;
            
            $collection = pq($e)->find('img');
            if( count($collection) > 0 )
            {
                foreach($collection as $item)
                    $config->globals["@autopush:paracollection"][] = "<image>" . pq($item)->attr('src');
            }
            else
            {
                $collection = pq($e)->find('video');
                if( count($collection) > 0 )
                {
                    foreach($collection as $item)
                        $config->globals["@autopush:paracollection"][] = "<video>" . pq($item)->find('source')->attr('src');
                }
                else
                {
                    $contents = trim(pq($e)->text());
                    if( ! empty($contents) )
                        $config->globals["@autopush:paracollection"][] = $contents;
                }
            }
        });
        
        $return = $config->globals["@autopush:paracollection"];
        unset( $config->globals["@autopush:paracollection"] );
        
        $config->globals["@autopush:messages_count"] = count($return);
        
        return $return;
    }
    
    /**
     * Extracts the slug from the permalink and looks it up in the database.
     * If found, returns a post record.
     * If not, returns the permalink.
     * 
     * @param string $permalink
     * @return string|post_record
     */
    private function get_post_from_permalink($permalink)
    {
        try
        {
            check_sql_injection($permalink);
        }
        catch(\Exception $e)
        {
            return $permalink;
        }
        
        static $repository = null;
        if( is_null($repository) ) $repository = new posts_repository();
        
        $parts = parse_url($permalink);
        $path  = $parts['path'];
        $parts = explode("/", $path);
        $slug  = end($parts);
        $post  = $repository->get_by_id_or_slug($slug);
        
        return is_null($post) ? $permalink : $post;
    }
}
