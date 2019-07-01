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

<style type="text/css">
    #autopush_options                                      { margin-bottom: 20px; }
    #autopush_options .endpoint_control                    { margin-top: 0; margin-bottom: 10px; }
    #autopush_options .endpoint_control:last-child         { margin-bottom: 0; }
    #autopush_options .endpoint_control .autopush-icon     { font-size: 1.2rem; }
    #autopush_options .endpoint_control .last_push_message ,
    #autopush_options .endpoint_control p                  { margin: 5px 0; }
</style>

<script type="text/javascript">
    function toggle_autopush_endpoint(trigger)
    {
        var $checkbox = $(trigger);
        var $parent   = $checkbox.closest('.endpoint_control');
        
        var ischecked = $checkbox.is(':checked');
        $parent.toggleClass('state_active', ischecked);
        
        if( ischecked )
        {
            if( $parent.find('input[type="radio"]:checked').length === 0 )
                $parent.find('input[type="radio"]:first').prop('checked', true);
        }
        else
        {
            $parent.find('input[type="radio"]:checked').prop('checked', false);
        }
        
        __toggle_message_textarea()
    }
    
    function enable_autopush_endpoint_state(trigger)
    {
        var $radio    = $(trigger);
        var $parent   = $radio.closest('.endpoint_control');
        var $checkbox = $parent.find('input[type="checkbox"]');
        
        if( $checkbox.is(':checked') )
        {
            __toggle_message_textarea();
            
            return;
        }
        
        $checkbox.prop('checked', true).trigger('change');
    }
    
    function __toggle_message_textarea()
    {
        var $message_textarea = $('#autopush_link_message_area');
        
        if( $('#autopush_options').find('input[type="radio"][value="as_link"]:checked').length > 0 )
            $message_textarea.show('fast');
        else
            $message_textarea.hide();
    }
    
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

<fieldset id="autopush_options">
    <legend><?= $this_module->language->autopush_to_socnet ?></legend>
    <div class="multicol cols-3">
        <?
        foreach($endpoints as $network => $collection):
            foreach($collection as $slug => $data):
                $class = ""; # $network == "twitter" ? "disabled"             : "";
                $style = ""; # $network == "twitter" ? "pointer-events: none" : "";
                ?>
                <div class="col framed_content endpoint_control <?= $class ?>" data-network="<?= $network ?>" style="<?= $style ?>">
                    <p>
                        <label class="alternate">
                            <input type="checkbox" name="autopush_to[<?= $network ?>][<?= $slug ?>][proceed]"
                                   value="true"
                                   onchange="toggle_autopush_endpoint(this)">
                            <i class="autopush-icon <?= $network ?>"></i>
                            <?= $data["title"] ?>
                        </label>
                    </p>
                    <div style="margin-left: 26px">
                        <div class="framed_content state_highlight last_push_message" style="display: none"></div>
                        <p>
                            <label>
                                <input type="radio" name="autopush_to[<?= $network ?>][<?= $slug ?>][method]"
                                       value="as_link" onchange="enable_autopush_endpoint_state(this)">
                                <?= $this_module->language->push_as_link ?>
                            </label>
                            <i class="fa fa-info-circle pseudo_link"
                               onclick="show_discardable_dialog('#autopush_aslink_info')"></i>
                        </p><p>
                            <label>
                                <input type="radio" name="autopush_to[<?= $network ?>][<?= $slug ?>][method]"
                                       value="as_pieces" onchange="enable_autopush_endpoint_state(this)">
                                <?= $this_module->language->push_as_pieces ?>
                            </label>
                            <i class="fa fa-info-circle pseudo_link"
                               onclick="show_discardable_dialog('#autopush_aspieces_info')"></i>
                        </p>
                    </div>
                </div>
                <?
            endforeach;
        endforeach;
        ?>
    </div>
    <div id="autopush_link_message_area" style="display: none;">
        <textarea name="autopush_link_message" style="width: 100%" rows="3" maxlength="250"
                  placeholder="<?= $this_module->language->link_message_placeholder ?>"></textarea>
    </div>
</fieldset>

<div id="autopush_aslink_info" style="display: none" title="<?= $this_module->language->aslink_info_dialog->title ?>">
    <?= $this_module->language->aslink_info_dialog->contents ?>
</div>

<div id="autopush_aspieces_info" style="display: none" title="<?= $this_module->language->aspieces_info_dialog->title ?>">
    <?= $this_module->language->aspieces_info_dialog->contents ?>
</div>
