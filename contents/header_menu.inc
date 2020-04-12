<?php
/**
 * Header menu buttons
 *
 * @package    BardCanvas
 * @subpackage Autopush
 * @author     Alejandro Caballero - lava.caballero@gmail.com
 *
 * @var account  $account
 * @var settings $settings
 * @var config   $config
 * @var template $template
 * @var module   $current_module anything
 * @var module   $this_module    self
 */

use hng2_base\account;
use hng2_base\config;
use hng2_base\module;
use hng2_base\settings;
use hng2_base\template;

global $config, $account, $settings, $account;

if( $account->level < $config::MODERATOR_USER_LEVEL ) return;
if( $current_module->name == "posts" && $template->layout == "admin" ) return;

$endpoints = $config->globals["@autopush:endpoints"];
if( empty($endpoints) ) return;

$caption = trim($this_module->language->push_message->button_caption);
$template->add_menu_item(
    $caption,
    "
    <span class='main_menu_item pseudo_link highlighted always_visible pull-right'
          data-module='autopush' data-priority='961' title='$caption'
          onclick=\"open_autopush_element_dialog('message:override')\">
        <i class='fa fa-bullhorn'></i>
    </span>
    ",
    959
);
