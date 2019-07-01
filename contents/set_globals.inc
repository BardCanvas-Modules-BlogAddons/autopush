<?php
/**
 * Pre-rendering globals
 *
 * @package    BardCanvas
 * @subpackage Autopush
 * @author     Alejandro Caballero - lava.caballero@gmail.com
 *
 * @var account  $account
 * @var settings $settings
 * @var config   $config
 * @var template $template
 */

use hng2_base\account;
use hng2_base\config;
use hng2_base\settings;
use hng2_base\template;

global $account, $settings, $config;

$config->globals["@autopush:endpoints"] = array();

$min_level = (int) $settings->get("modules:posts.level_allowed_to_edit_custom_fields");
if( empty($min_level) ) $min_level = $config::MODERATOR_USER_LEVEL;
if( $account->level < $min_level ) return;

$raw_endpoints = $settings->get("modules:autopush.endpoints");
if( empty($raw_endpoints) ) return;

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
}
if( empty($endpoints) ) return;

$config->globals["@autopush:endpoints"] = $endpoints;
