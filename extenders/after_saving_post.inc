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

use hng2_modules\autopush\toolbox;
use hng2_modules\posts\post_record;

global $post, $config, $account, $settings;

if( empty($post->id_post) ) return;
if( $_POST["is_autosave"] == "true" ) return;
if( empty($post->publishing_date) ) return;
if( $post->publishing_date == "0000-00-00 00:00:00" ) return;
if( $post->status != "published" ) return;

$min_level = (int) $settings->get("modules:posts.level_allowed_to_edit_custom_fields");
if( empty($min_level) ) $min_level = $config::MODERATOR_USER_LEVEL;
if( $account->level < $min_level ) return;

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

# Initial prechecks
if( empty($_POST["autopush_to"]) ) return;
if( ! is_array($_POST["autopush_to"]) ) return;

# Post visibility validations
if( $post->visibility != "public" || $post->password != "" )
{
    send_notification($account->id_account, "warning", unindent($this_module->language->messages->post_not_public));
    return;
}
if( $post->main_category_visibility != "public" || $post->main_category_min_level > 0 )
{
    send_notification($account->id_account, "warning", unindent($this_module->language->messages->post_category_not_public));
    return;
}

#
# Main loop
#

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
            send_notification(
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
            send_notification(
                $account->id_account,
                "warning",
                sprintf($this_module->language->messages->invalid_method, $method, $title)
            );
            sleep(1);
            
            continue;
        }
        
        # Reference inits (not mandatory, actually reset by the push method)
        $config->globals["@autopush:sending_errors"] = array();
        $config->globals["@autopush:messages_count"] = 0;
        $config->globals["@autopush:messages_sent"]  = 0;
        
        # Actual push
        $toolbox->push(
            $network, $method, $endpoints[$network][$slug], $post, $account, true,
            trim(stripslashes($_POST["autopush_link_message"]))
        );
        
        if( ! empty($config->globals["@autopush:sending_errors"]) ) continue;
        
        # Notify
        send_notification(
            $account->id_account, "success",
            unindent(sprintf(
                $this_module->language->messages->push_ok,
                ucwords($network),
                $title,
                $config->globals["@autopush:messages_sent"],
                $config->globals["@autopush:messages_count"]
            ))
        );
        sleep( 1 );
    }
}
