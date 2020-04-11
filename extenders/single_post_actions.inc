<?php
/**
 * Posts extender: single post actions
 *
 * @package    BardCanvas
 * @subpackage Autopush
 * @author     Alejandro Caballero - lava.caballero@gmail.com
 */

global $config, $account, $settings, $post;

$endpoints = $config->globals["@autopush:endpoints"];

if( empty($endpoints) ) return;
if( empty($post) ) return;

$min_level = (int) $settings->get("modules:posts.level_allowed_to_edit_custom_fields");
if( empty($min_level) ) $min_level = $config::MODERATOR_USER_LEVEL;
if( $account->level < $min_level ) return;
if( $account->level < $config::MODERATOR_USER_LEVEL ) return;

if( $post->status != "published" ) return;
if( $post->visibility != "public" ) return;
if( ! empty($post->password) ) return;
if( $post->main_category_visibility != "public" ) return;
?>

<!-- Autopush button -->

<style type="text/css">
    .single_item_actions .action.autopush {
        border-color:     #542a13;
        color:            white;
        background-color: #bf6000;
    }

    .single_item_actions .action.autopush:hover {
        border-color:     saddlebrown;
        color:            white;
        background-color: darkorange;
    }
</style>

<span class="item action autopush" onclick="open_autopush_element_dialog( '<?= $post->get_permalink(true) ?>' )">
    <span class="fa fa-share-alt"></span>
    <?= $this_module->language->single_post_actions->push ?>
</span>

