<?php
/**
 * Posts extender: additions to form below the editor
 *
 * @package    BardCanvas
 * @subpackage Autopush
 * @author     Alejandro Caballero - lava.caballero@gmail.com
 */

global $config, $account, $settings;

$endpoints = $config->globals["@autopush:endpoints"];

if( empty($endpoints) ) return;
?>

<!-- Autopush to social networks -->

<link rel="stylesheet" type="text/css" href="<?= $this_module->get_url() ?>/media/form_styles~v1.0.1.css">
<script type="text/javascript" src="<?= $this_module->get_url() ?>/media/form_scripts~v1.0.0.js"></script>

<script type="text/javascript">
    //
    // Extenders for the post form filling/reset methods
    //
    
    if( typeof fill_post_form_extensions === 'undefined' )
        fill_post_form_extensions = {};
    
    fill_post_form_extensions['autopush_check_previous_pushes'] = function($form, record)
    {
        if( typeof record.custom_fields == 'undefined' ) return;
        
        var $container = $('#autopush_options');
        
        for(var i in record.custom_fields)
        {
            if( i.indexOf('@autopush:') < 0 ) continue;
            if( i.indexOf('.last_push') < 0 ) continue;
            
            var network = i.replace('@autopush:', '').replace('.last_push', '');
            var message = record.custom_fields[i];
            
            var $target = $container.find(sprintf('.endpoint_control[data-network="%s"]', network));
            if( $target.length === 0 ) continue;
            
            $target.find('.last_push_message')
                   .html(sprintf('<i class="fa fa-info-circle"></i> %s', message))
                   .show()
        }
    };
    
    if( typeof reset_post_form_extensions === 'undefined' )
        reset_post_form_extensions = {};

    reset_post_form_extensions['autopush_reset_form'] = function($form)
    {
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
    };
</script>

<?
include __DIR__ . "/post_form_fields.inc";
$config->globals["@autopush:form_already_rendered"] = true;
?>
