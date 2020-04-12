<?php
/**
 * Post-footer scripts
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

global $config, $account, $settings;

if( $config->globals["@autopush:form_already_rendered"] ) return;

$endpoints = $config->globals["@autopush:endpoints"];

if( empty($endpoints) ) return;
?>

<!-- Autopush to social networks "push element" helper stuff -->

<link rel="stylesheet" type="text/css" href="<?= $this_module->get_url() ?>/media/form_styles~v1.0.1.css">

<script type="text/javascript" src="<?= $this_module->get_url() ?>/media/form_scripts~v1.0.0.js"></script>

<style type="text/css">
    #autopush_options                { margin: 0; }
    #autopush_options .multicol      { display: block; width: 100%; }
    #autopush_options .multicol .col { display: block; width: 100%; }

    #autopush_options .endpoint_radios { display: none; }
</style>

<div id="autopush_element_target" style="display: none"></div>

<div id="autopush_element_dialog" style="display: none"
     title="<?= $this_module->language->dialog->title ?>"
     data-link-title="<?= $this_module->language->dialog->title ?>"
     data-msg-title="<?= $this_module->language->push_message->alt_dialog_title ?>"
     data-ok-caption="<?= $language->submit ?>" data-cancel-caption="<?= $language->cancel ?>">
    
    <form id="autopush_element_form" method="post" action="<?= $this_module->get_url() ?>/scripts/push_element.php">
        
        <div class="field" for="url">
            <div class="caption"><?= $this_module->language->dialog->url->caption ?></div>
            <div class="input">
                <input type="text" name="url" placeholder="<?= $this_module->language->dialog->url->placeholder ?>">
            </div>
        </div>
        
        <?
        include "{$this_module->abspath}/extenders/post_form_fields.inc";
        $config->globals["@autopush:form_already_rendered"] = true;
        ?>
        
    </form>
    
</div>

<script type="text/javascript">
    
    function open_autopush_element_dialog(url)
    {
        var $form     = $('#autopush_element_form');
        var $dialog   = $('#autopush_element_dialog');
        var $textarea = $('#autopush_link_message_area textarea[name="autopush_link_message"]');
        var title, textarea_placeholder;
        
        if( url === 'message:override' )
        {
            $form.find('.field[for="url"]').hide();
            title = $dialog.attr('data-msg-title');
            textarea_placeholder = $textarea.attr('data-msg-placeholder');
        }
        else
        {
            $form.find('.field[for="url"]').show();
            title = $dialog.attr('data-link-title');
            textarea_placeholder = $textarea.attr('data-link-placeholder');
        }
        
        $textarea.attr('placeholder', textarea_placeholder);
        $form.find('input[name="url"]').val( url );
        $dialog.dialog('option', 'title', title);
        $dialog.dialog('open');
    }
    
    function autopush_element_prepare(data, $form)
    {
        $form.closest('.ui-dialog').block(blockUI_default_params);
    }
    
    function autopush_element_success(response, status, xhr, $form)
    {
        $form.closest('.ui-dialog').unblock();
        
        if( response !== 'OK')
        {
            alert(response);
            
            return;
        }
        
        $('#autopush_element_dialog').dialog('close');
    }
    
    function autopush_element_dialog_reset()
    {
        $('#autopush_element_form').find('input[name="url"]').val('');
        
        var $fieldset = $('#autopush_options');
        
        $fieldset
            .find('input[type="checkbox"]')
            .prop('checked', false)
            .trigger('change');
        
        $fieldset
            .find('.last_push_message')
            .html('')
            .hide();
        
        $('#autopush_link_message_area').hide();
        $('#autopush_link_message').val('');
    }
    
    $(document).ready(function()
    {
        var $dialog    = $('#autopush_element_dialog');
        var $form      = $('#autopush_element_form');
        var width  = $(window).width()  - 20;
        var height = $(window).height() - 20;
        
        if( width  > 600 ) width  = 600;
        
        $dialog.dialog({
            modal:     true,
            autoOpen:  false,
            width:     width,
            maxHeight: height,
            close:     autopush_element_dialog_reset,
            buttons:   [
                {
                    text:  $dialog.attr('data-cancel-caption'),
                    icons: { primary: "ui-icon-cancel" },
                    click: function() { $(this).dialog( "close" ); }
                },
                {
                    text:  $dialog.attr('data-ok-caption'),
                    icons: { primary: "ui-icon-check" },
                    click: function() { $('#autopush_element_form').submit(); }
                }
            ]
        });
        
        $form.ajaxForm({
            target:       '#autopush_element_target',
            beforeSubmit: autopush_element_prepare,
            success:      autopush_element_success
        });
    });
    
</script>
