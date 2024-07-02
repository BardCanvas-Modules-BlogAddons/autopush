<?php
/**
 * Autopush single element.
 * Note: only users that can edit custom post fields can push anything.
 *
 * @package    BardCanvas
 * @subpackage Autopush
 * @author     Alejandro Caballero - lava.caballero@gmail.com
 * 
 * $_POST params:
 * @param string url                   Either a fully qualified URL or "message:override" for pushing a message.
 * @param array  autopush_to           Targets collection.
 * @param string autopush_link_message Optional message for link. Mandatory if url is "message:override"
 */

use hng2_modules\autopush\toolbox;
use hng2_tools\cli_colortags;

include "../../config.php";
include "../../includes/bootstrap.inc";
header("Content-Type: text/plain; charset=UTF-8");

$min_level = (int) $settings->get("modules:posts.level_allowed_to_edit_custom_fields");
if( empty($min_level) ) $min_level = $config::MODERATOR_USER_LEVEL;
if( $account->level < $min_level ) throw_fake_401();

# Initial prechecks
$url = trim(stripslashes($_POST["url"]));
if( empty($url) ) die($current_module->language->messages->missing_url);

if( $url == "message:override" && $account->level < $config::MODERATOR_USER_LEVEL )
    die($current_module->language->push_message->only_mods_can_push_messages);
elseif( $url != "message:override" && ! filter_var($url, FILTER_VALIDATE_URL) )
    die($current_module->language->messages->invalid_url);

if( empty($_POST["autopush_to"]) )            die($current_module->language->messages->no_endpoints_selected);
if( ! is_array($_POST["autopush_to"]) )       die($current_module->language->messages->no_endpoints_selected);

# Endpoints pre-validation
$raw_endpoints = $settings->get("modules:autopush.endpoints");
if( empty($raw_endpoints) ) die($current_module->language->messages->no_endpoints_defined);

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
        
        $endpoints["telegram"][$slug]["title"]        = $parts[0];
        $endpoints["telegram"][$slug]["token"]        = $parts[1];
        $endpoints["telegram"][$slug]["target"]       = $parts[2];
    }
    elseif( substr($line, 0, 17) == "linkedin_profile:" )
    {
        $line  = substr($line, 18);
        $parts = explode(", ", $line);
        
        if( empty($parts[0]) ) continue;
        if( empty($parts[1]) ) continue;
        if( empty($parts[2]) ) continue;
        
        $slug  = wp_sanitize_filename($parts[0]);
        
        $endpoints["linkedin_profile"][$slug]["title"]  = $parts[0];
        $endpoints["linkedin_profile"][$slug]["token"]  = $parts[1];
        $endpoints["linkedin_profile"][$slug]["target"] = $parts[2];
    }
    elseif( substr($line, 0, 17) == "linkedin_company:" )
    {
        $line  = substr($line, 18);
        $parts = explode(", ", $line);
        
        if( empty($parts[0]) ) continue;
        if( empty($parts[1]) ) continue;
        if( empty($parts[2]) ) continue;
        
        $slug  = wp_sanitize_filename($parts[0]);
        
        $endpoints["linkedin_company"][$slug]["title"]  = $parts[0];
        $endpoints["linkedin_company"][$slug]["token"]  = $parts[1];
        $endpoints["linkedin_company"][$slug]["target"] = $parts[2];
    }
}
if( empty($endpoints) ) die($current_module->language->messages->no_endpoints_defined);

#
# Main loop
#

$logfile = sprintf("%s/autopush_submissions-%s.log", $config->logfiles_location, date("Ymd"));
cli_colortags::$output_file = $logfile;
cli_colortags::$output_to_file_only = true;

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
        $method = $url == "message:override" ? "as_link" : $options["method"];
        
        # Method validation
        if( ! in_array($method, array("as_link", "as_pieces")) )
        {
            send_notification(
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
        $link_message = trim(stripslashes($_POST["autopush_link_message"]));
        
        # Actual push
        $toolbox->push(
            $network, $method, $endpoints[$network][$slug], $url, $account, true,
            $link_message
        );
        
        # Log and notify
        $lmethod = $method == "as_link" ? "link" : "text";
        $lwhat   = $url == "message:override" ? make_excerpt_of($link_message) : $url;
        $logdate = date("Y-m-d H:i:s");
        cli_colortags::write(
            "[$logdate] - <light_blue>{$account->display_name}</light_blue> " .
            "pushing <light_cyan>$lmethod</light_cyan> " .
            "<light_blue>$lwhat</light_blue> " .
            "to <white>$network/$slug</white>... "
        );
        if( empty($config->globals["@autopush:sending_errors"]) )
        {
            cli_colortags::write(
                "<green>OK!</green>\n"
            );
            send_notification(
                $account->id_account, "success",
                unindent(sprintf(
                    $this_module->language->messages->push_ok2,
                    ucwords($network),
                    $title,
                    $config->globals["@autopush:messages_sent"],
                    $config->globals["@autopush:messages_count"]
                ))
            );
        }
        else
        {
            cli_colortags::write(
                "\n<red>   Errors detected!</red>\n"
            );
            foreach($config->globals["@autopush:sending_errors"] as $error) cli_colortags::write(
                "   <light_purple>~ $error</light_purple>\n"
            );
        }
    }
}

echo "OK";
