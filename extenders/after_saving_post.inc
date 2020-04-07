<?php
/**
 * Posts extender: if published and not pushed, push it.
 *
 * @var post_record $post
 *
 * @package    BardCanvas
 * @subpackage Autopush
 * @author     Alejandro Caballero - lava.caballero@gmail.com
 * 
 * $_POST params:
 * @param array "autopush_to" [network][slug][proceed|method]
 */

use hng2_base\account;
use hng2_modules\autopush\toolbox;
use hng2_modules\categories\categories_repository;
use hng2_modules\posts\post_record;
use hng2_tools\cli_colortags;

global $post, $old_post, $config, $account, $settings;

if( empty($post->id_post) ) return;
if( $_POST["is_autosave"] == "true" ) return;
if( empty($post->publishing_date) ) return;
if( $post->publishing_date == "0000-00-00 00:00:00" ) return;
if( $post->status != "published" ) return;

# Endpoints pre-validation
$raw_endpoints = $settings->get("modules:autopush.endpoints");
if( empty($raw_endpoints) ) return;

# Endpoints parsing
$endpoints = array();
foreach(explode("\n", $raw_endpoints) as $line)
{
    $line = trim($line);
    if( empty($line) ) continue;
    
    if( substr($line, 0, 8) == "twitter:" )
    {
        $line  = substr($line, 9);
        $parts = explode(", ", $line);
        
        if( empty($parts[0]) ) continue;
        if( empty($parts[1]) ) continue;
        if( empty($parts[2]) ) continue;
        if( empty($parts[3]) ) continue;
        if( empty($parts[4]) ) continue;
        
        $slug = wp_sanitize_filename($parts[0]);
        
        $endpoints["twitter"][$slug]["title"]        = $parts[0];
        $endpoints["twitter"][$slug]["api_key"]      = $parts[1];
        $endpoints["twitter"][$slug]["api_secret"]   = $parts[2];
        $endpoints["twitter"][$slug]["token_key"]    = $parts[3];
        $endpoints["twitter"][$slug]["token_secret"] = $parts[4];
    }
    elseif( substr($line, 0, 8) == "discord:" )
    {
        $line  = substr($line, 9);
        $parts = explode(", ", $line);
        
        if( empty($parts[0]) ) continue;
        if( empty($parts[1]) ) continue;
        
        $slug  = wp_sanitize_filename($parts[0]);
        
        $endpoints["discord"][$slug]["title"]        = $parts[0];
        $endpoints["discord"][$slug]["webhook"]      = $parts[1];
    }
    elseif( substr($line, 0, 9) == "telegram:" )
    {
        $line  = substr($line, 10);
        $parts = explode(", ", $line);
        
        if( empty($parts[0]) ) continue;
        if( empty($parts[1]) ) continue;
        if( empty($parts[2]) ) continue;
        
        $slug  = wp_sanitize_filename($parts[0]);
        
        $endpoints["telegram"][$slug]["title"]       = $parts[0];
        $endpoints["telegram"][$slug]["token"]       = $parts[1];
        $endpoints["telegram"][$slug]["target"]      = $parts[2];
    }
}
if( empty($endpoints) ) return;

#
# Level / automated push directives validation
#

$notifications_target = "current_user";
$cats_repo = new categories_repository();

$min_level = (int) $settings->get("modules:posts.level_allowed_to_edit_custom_fields");
if( empty($min_level) ) $min_level = $config::MODERATOR_USER_LEVEL;
if( $account->level < $min_level )
{
    $raw_directives = $settings->get("modules:autopush.automated_push_directives");
    if( empty($raw_directives) ) return;
    
    $_POST["autopush_to"]           = array();
    $_POST["autopush_link_message"] = "";
    
    $allow_pushing = false;
    foreach(explode("\n", $raw_directives) as $line)
    {
        $line = trim($line);
        
        #
        # Primary directive forging and validations
        #
        
        if( empty($line) ) continue;
        if( substr($line, 0, 1) == "#" ) continue;
        
        $parts = explode(":", $line);
        if( count($parts) != 2 ) continue;
        
        $endpoint_title = trim($parts[0]);
        $category_slug  = trim($parts[1]);
        
        $parts = explode("/", $endpoint_title);
        if( count($parts) != 2 ) continue;
        
        $network_name   = strtolower(trim($parts[0]));
        $endpoint_title = trim($parts[1]);
        $endpoint_slug  = wp_sanitize_filename($endpoint_title);
        
        if( ! isset($endpoints[$network_name][$endpoint_slug]) ) continue;
        
        #
        # Directive category validations
        #
        
        $category_ids   = array();
        $category_slugs = explode(",", $category_slug);
        foreach($category_slugs as $category_slug)
        {
            $category_id = $cats_repo->get_id_by_slug($category_slug);
            if( ! empty($category_id) ) $category_ids[] = $category_id;
        }
        
        if( empty($category_ids) ) continue;
        
        if( in_array($post->main_category, $category_ids) )
        {
            $allow_pushing = true;
            $_POST["autopush_to"][$network_name][$endpoint_slug]["proceed"] = "true";
            $_POST["autopush_to"][$network_name][$endpoint_slug]["method"]  = "as_link";
        }
    }
    
    if( ! $allow_pushing ) return;
    
    $notifications_target = "logfile";
    
    $auto_caption = $settings->get("modules:autopush.automated_push_caption");
    if( empty($auto_caption) )
    {
        $_POST["autopush_link_message"] = strip_tags($post->excerpt);
        if( empty($_POST["autopush_link_message"]) )
            $_POST["autopush_link_message"] = strip_tags($post->title);
    }
    else
    {
        $post_author   = new account($post->id_author);
        $post_category = $cats_repo->get($post->main_category);
        
        $_POST["autopush_link_message"] = replace_escaped_objects($auto_caption, array(
            '{$user_name}' => $post_author->display_name,
            '{$category}'  => $post_category->title,
        ));
    }
}

# Initial prechecks
if( empty($_POST["autopush_to"]) ) return;
if( ! is_array($_POST["autopush_to"]) ) return;

# Post visibility validations
# Note: only sent to the current user (custom fields allowed).
# If automated push directives are green (normal user), mods/admins aren't notified to avoid spam.
if( $post->visibility != "public" || $post->password != "" )
{
    if( $notifications_target == "user" ) send_notification(
        $account->id_account, "warning", unindent($this_module->language->messages->post_not_public)
    );
    
    return;
}
if( $post->main_category_visibility != "public" || $post->main_category_min_level > 0 )
{
    if( $notifications_target == "user" ) send_notification(
        $account->id_account, "warning", unindent($this_module->language->messages->post_category_not_public)
    );
    return;
}

#
# Main loop
#

$logfile = sprintf("%s/autopush_submissions-%s.log", $config->logfiles_location, date("Ymd"));
cli_colortags::$output_file = $logfile;
cli_colortags::$output_to_file_only = true;

$post_meta = null;
if( ! is_null($old_post) )
    $post_meta = $old_post->fetch_all_metas();

$toolbox = new toolbox();
foreach($_POST["autopush_to"] as $network => $slugs )
{
    if( ! is_array($slugs) ) continue;
    
    foreach($slugs as $slug => $options)
    {
        # Options validation
        if( ! is_array($options) ) continue;
        
        # Endpoint validations
        if( ! isset($endpoints[$network][$slug]) )
        {
            if( $notifications_target == "user" ) send_notification(
                $account->id_account,
                "warning",
                sprintf($this_module->language->messages->invalid_endpoint, $slug)
            );
            
            continue;
        }
        
        # Proceeding validation
        if( $options["proceed"] != "true" ) continue;
        
        # Some inits
        $title  = $endpoints[$network][$slug]["title"];
        $method = $options["method"];
        
        # Method validation
        if( ! in_array($method, array("as_link", "as_pieces")) )
        {
            if( $notifications_target == "user" ) send_notification(
                $account->id_account,
                "warning",
                sprintf($this_module->language->messages->invalid_method, $method, $title)
            );
            
            continue;
        }
        
        # Reference inits (not mandatory, actually reset by the push method)
        $config->globals["@autopush:sending_errors"] = array();
        $config->globals["@autopush:messages_count"] = 0;
        $config->globals["@autopush:messages_sent"]  = 0;
        
        # Precheck for being already pushed
        if(
            is_array($post_meta)
            && isset($post_meta["@autopush:$network.last_push"])
            && $notifications_target != "user"
        ) {
            cli_colortags::write(sprintf(
                "<purple>[%s] Already pushed to %s/%s:</purple>\n" .
                "<purple> • Post #%s (%s) on %s by %s (%s):</purple>\n" .
                "<light_purple> > %s</light_purple>\n",
                date("Y-m-d H:i:s"),
                $network,
                $slug,
                $post->id_post,
                $post->title,
                $post_category->title,
                $post_author->user_name,
                $post_author->display_name,
                str_replace(array("\r", "\n"), " ", unindent($post_meta["@autopush:$network.last_push"]))
            ));
            
            continue;
        }
        
        # Actual push
        $toolbox->push(
            $network, $method, $endpoints[$network][$slug], $post, $account, true,
            trim(stripslashes($_POST["autopush_link_message"]))
        );
        
        if( ! empty($config->globals["@autopush:sending_errors"]) )
        {
            if( $notifications_target == "user" ) continue;
            
            cli_colortags::write(sprintf(
                "<light_red>[%s] Errors attempting automated push to %s/%s:</light_red>\n" .
                "<light_red> • Post #%s (%s) on %s by %s (%s):</light_red>\n",
                date("Y-m-d H:i:s"),
                $network,
                $slug,
                $post->id_post,
                $post->title,
                $post_category->title,
                $post_author->user_name,
                $post_author->display_name
            ));
            foreach($config->globals["@autopush:sending_errors"] as $error)
                cli_colortags::write("<light_red> • $error</light_red>\n");
            cli_colortags::write("\n");
            
            continue;
        }
        
        # Notify
        $notification = unindent(sprintf(
            $this_module->language->messages->push_ok,
            ucwords($network),
            $title,
            $config->globals["@autopush:messages_sent"],
            $config->globals["@autopush:messages_count"]
        ));
        
        if( $notifications_target == "user" )
        {
            send_notification($account->id_account, "success", $notification);
        }
        else
        {
            $post_author   = new account($post->id_author);
            $post_category = $cats_repo->get($post->main_category);
            
            if( $method == "as_link" )
                cli_colortags::write(sprintf(
                    "[%s] Pushed <light_cyan>link</light_cyan> to <white>%s/%s</white>:\n" .
                    " • Post #%s (%s) on %s by %s (%s)\n\n",
                    date("Y-m-d H:i:s"),
                    $network,
                    $slug,
                    $post->id_post,
                    $post->title,
                    $post_category->title,
                    $post_author->user_name,
                    $post_author->display_name
                ));
            else
                cli_colortags::write(sprintf(
                    "[%s] Pushed <light_blue>post contents</light_blue> to <white>%s/%s</white>:\n" .
                    " • Post #%s (%s) on %s by %s (%s)\n\n",
                    date("Y-m-d H:i:s"),
                    $network,
                    $slug,
                    $post->id_post,
                    $post->title,
                    $post_category->title,
                    $post_author->user_name,
                    $post_author->display_name
                ));
        }
    }
}
