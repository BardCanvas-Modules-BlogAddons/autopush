<?php
/**
 * Settings field type extenders: preinits
 *
 * @package    BardCanvas
 * @subpackage Autopush
 * @author     Alejandro Caballero - lava.caballero@gmail.com
 * 
 * Trailing globals:
 * @var module[] $modules
 * @var module   $current_module settings
 * @var module   $this_module    self
 */

use hng2_base\module;

global $settings, $template;

$encryption_key = "bfxFUbgQbQ7QNCmVaK";
?>

<!-- Autopush Scripts -->

<style type="text/css">
    #autopush_endpoints_container .target                        { max-height: 60vh; overflow: auto; padding: 5px;
                                                                   border: 2px inset silver; border-radius: 5px;
                                                                   margin-bottom: 10px; }
    #autopush_endpoints_container .target fieldset               { margin-bottom: 10px; }
    #autopush_endpoints_container .target fieldset:last-child    { margin-bottom: 0; }
    #autopush_endpoints_container .target fieldset legend        { padding: 0 5px; }
    #autopush_endpoints_container .target fieldset legend button { font-size: 7pt; padding: 0 3px; text-transform: uppercase; vertical-align: middle; }
    
    #autopush_endpoints_container .target fieldset[data-network="twitter"]        { border-color: #28aae1; background-color: #e8f6fc; }
    #autopush_endpoints_container .target fieldset[data-network="twitter"] legend { color: #28aae1; background-color: white; 
                                                                                    border: 2px outset #28aae1; border-radius: 5px; }
    #autopush_endpoints_container .target fieldset[data-network="discord"]        { border-color: #7289da; background-color: #f0f2fb; }
    #autopush_endpoints_container .target fieldset[data-network="discord"] legend { color: #7289da; background-color: white;
                                                                                    border: 2px outset #7289da; border-radius: 5px; }
    #autopush_endpoints_container .target fieldset[data-network="telegram"]        { border-color: #7289da; background-color: #f0f2fb; }
    #autopush_endpoints_container .target fieldset[data-network="telegram"] legend { color: #0D86D7; background-color: white;
                                                                                     border: 2px outset #0D86D7; border-radius: 5px; }
    
    #autopush_endpoints_container .target fieldset[data-network="linkedin_profile"]        { border-color: #0077b7; background-color: #F0F7FB; }
    #autopush_endpoints_container .target fieldset[data-network="linkedin_profile"] legend { color: #0077b7; background-color: white;
                                                                                             border: 2px outset #0077b7; border-radius: 5px; }
    
    #autopush_endpoints_container .target fieldset[data-network="linkedin_company"]        { border-color: #992e96; background-color: #F9F4F9; }
    #autopush_endpoints_container .target fieldset[data-network="linkedin_company"] legend { color: #992e96; background-color: white;
                                                                                             border: 2px outset #992e96; border-radius: 5px; }
    
    #autopush_endpoints_container .target fieldset.new { box-shadow: 0 0 5px green; }
</style>

<script type="text/javascript">
    
    function add_autopush_template(key)
    {
        if( key === '' ) return;
        
        var html    = $('#autopush_endpoint_templates').find(sprintf('template.%s', key)).html();
        var $target = $('#autopush_endpoints_container').find('.target');
        var $item   = $(html);
        $item.find('.expandible_textarea').expandingTextArea();
        $target.append($item);
        $target.scrollTo('max', 'fast');
        
        $('#add_endpoint_selector').find('option:first').prop('selected', true);
    }
    
    function remove_autopush_endpoint(trigger)
    {
        if( ! confirm($_GENERIC_CONFIRMATION) ) return;
        
        $(trigger).closest('fieldset').hide('fast', function()
        {
            $(this).remove();
            rebuild_autopush_endpoints();
        })
    }
    
    function rebuild_autopush_endpoints()
    {
        var $container = $('#autopush_endpoints_container');
        var $textarea  = $container.find('textarea.target_value');
        var options    = [];
        
        $container.find('.target fieldset').each(function() {
            var $this   = $(this);
            var network = $this.attr('data-network');
            switch( network )
            {
                case 'twitter':
                    options.push(sprintf(
                        '%s: %s, %s, %s, %s, %s',
                        network,
                        $this.find('input[data-field="title"]').val().replace(/,:'"/, '').trim(),
                        $this.find('input[data-field="consumer_key"]').val().trim(),
                        $this.find('input[data-field="consumer_secret"]').val().trim(),
                        $this.find('input[data-field="token"]').val().trim(),
                        $this.find('input[data-field="token_secret"]').val().trim()
                    ));
                    break;
                case 'discord':
                    options.push(sprintf(
                        '%s: %s, %s',
                        network,
                        $this.find('input[data-field="title"]').val().replace(/,:'"/, '').trim(),
                        $this.find('input[data-field="webhook"]').val().trim()
                    ));
                    break;
                case 'telegram':
                    options.push(sprintf(
                        '%s: %s, %s, %s',
                        network,
                        $this.find('input[data-field="title"]').val().replace(/,:'"/, '').trim(),
                        $this.find('input[data-field="token"]').val().trim().replace(/\s/g, ''),
                        $this.find('input[data-field="target"]').val().trim()
                    ));
                    break;
                case 'linkedin_profile':
                case 'linkedin_company':
                    options.push(sprintf(
                        '%s: %s, %s, %s',
                        network,
                        $this.find('input[data-field="title"]').val().replace(/,:'"/, '').trim(),
                        $this.find('textarea[data-field="token"]').val().trim().replace(/\s/g, ''),
                        $this.find('input[data-field="target"]').val().trim()
                    ));
                    break;
            }
        });
        
        if( options.length > 0 ) $textarea.val(options.join('\n'));
        else                     $textarea.val('');
    }
    
</script>

<div id="autopush_endpoint_templates" style="display: none">
    
    <template class="twitter">
        <fieldset class="new" data-network="twitter">
            <legend>
                <i class="autopush-icon twitter"></i>
                <button onclick="$(this).closest('fieldset').hide('fast', function() { $(this).remove(); }); return false;">
                    <i class="fa fa-trash"></i>
                    <?= $this_module->language->remove ?>
                </button>
            </legend>
            <table class="nav_table_inner">
                <? foreach(array("title", "consumer_key", "consumer_secret", "token", "token_secret") as $field): ?>
                    <tr>
                        <th nowrap><?= $this_module->language->new->twitter->fields->{$field}->caption ?></th>
                        <td width="100%">
                            <input type="text" data-field="<?= $field ?>"
                                   placeholder="<?= $this_module->language->new->twitter->fields->{$field}->placeholder ?>"
                                   style="width: 100%" onblur="rebuild_autopush_endpoints()">
                        </td>
                    </tr>
                <? endforeach; ?>
            </table>
        </fieldset>
    </template>
    
    <template class="discord">
        <fieldset class="new" data-network="discord">
            <legend>
                <i class="autopush-icon discord"></i>
                <?= $this_module->language->new->discord->legend ?>
                <button onclick="$(this).closest('fieldset').hide('fast', function() { $(this).remove(); }); return false;">
                    <i class="fa fa-trash"></i>
                    <?= $this_module->language->remove ?>
                </button>
            </legend>
            <table class="nav_table_inner">
                <? foreach(array("title", "webhook") as $field): ?>
                    <tr>
                        <th nowrap><?= $this_module->language->new->discord->fields->{$field}->caption ?></th>
                        <td width="100%">
                            <input type="text" data-field="<?= $field ?>"
                                   placeholder="<?= $this_module->language->new->discord->fields->{$field}->placeholder ?>"
                                   style="width: 100%" onblur="rebuild_autopush_endpoints()">
                        </td>
                    </tr>
                <? endforeach; ?>
            </table>
        </fieldset>
    </template>
    
    <template class="telegram">
        <fieldset class="new" data-network="telegram">
            <legend>
                <i class="autopush-icon telegram"></i>
                <?= $this_module->language->new->telegram->legend ?>
                <button onclick="$(this).closest('fieldset').hide('fast', function() { $(this).remove(); }); return false;">
                    <i class="fa fa-trash"></i>
                    <?= $this_module->language->remove ?>
                </button>
            </legend>
            <table class="nav_table_inner">
                <? foreach(array("title", "token", "target") as $field): ?>
                    <tr>
                        <th nowrap><?= $this_module->language->new->telegram->fields->{$field}->caption ?></th>
                        <td width="100%">
                            <input type="text" data-field="<?= $field ?>"
                                   placeholder="<?= $this_module->language->new->telegram->fields->{$field}->placeholder ?>"
                                   style="width: 100%" onblur="rebuild_autopush_endpoints()">
                        </td>
                    </tr>
                <? endforeach; ?>
            </table>
        </fieldset>
    </template>
    
    <?
    $sp_url  = $config->full_root_url;
    $sp_name = $settings->get('engine.website_name');
    $sp_icon = $settings->get('engine.favicon');
    if( empty($sp_icon) ) $sp_icon = $template->get("favicon");
    if( empty($sp_icon) ) $sp_icon = "none";
    ?>
    
    <template class="linkedin_profile">
        <fieldset class="new" data-network="linkedin_profile">
            <legend>
                <i class="autopush-icon linkedin_profile"></i>
                <?= $this_module->language->new->linkedin_profile->legend ?>
                <button onclick="$(this).closest('fieldset').hide('fast', function() { $(this).remove(); }); return false;">
                    <i class="fa fa-trash"></i>
                    <?= $this_module->language->remove ?>
                </button>
            </legend>
            
            <div class="framed_content state_highlight" style="margin: 0 0 5px 0;">
                <?= replace_escaped_objects($this_module->language->new->linkedin_profile->info, array(
                    '{$link}' => sprintf(
                        "https://bardcanvas.com/autopush-for-linkedin-authorization?start=%s",
                        urlencode(encrypt("profile\t{$sp_url}\t{$sp_name}\t{$sp_icon}", $encryption_key))
                    )
                )) ?>
            </div>
            
            <table class="nav_table_inner">
                <? foreach(array("title", "token", "target") as $field): ?>
                    <tr>
                        <th nowrap><?= $this_module->language->new->linkedin_profile->fields->{$field}->caption ?></th>
                        <td width="100%">
                            <? if( $field == "token" ): ?>
                                <textarea class="expandible_textarea" data-field="<?= $field ?>"
                                       placeholder="<?= $this_module->language->new->linkedin_profile->fields->{$field}->placeholder ?>"
                                       onblur="rebuild_autopush_endpoints()"></textarea>
                            <? else: ?>
                                <input type="text" data-field="<?= $field ?>"
                                       placeholder="<?= $this_module->language->new->linkedin_profile->fields->{$field}->placeholder ?>"
                                       style="width: 100%" onblur="rebuild_autopush_endpoints()">
                            <? endif; ?>
                        </td>
                    </tr>
                <? endforeach; ?>
            </table>
        </fieldset>
    </template>
    
    <template class="linkedin_company">
        <fieldset class="new" data-network="linkedin_company">
            <legend>
                <i class="autopush-icon linkedin_company"></i>
                <?= $this_module->language->new->linkedin_company->legend ?>
                <button onclick="$(this).closest('fieldset').hide('fast', function() { $(this).remove(); }); return false;">
                    <i class="fa fa-trash"></i>
                    <?= $this_module->language->remove ?>
                </button>
            </legend>

            <div class="framed_content state_highlight" style="margin: 0 0 5px 0;">
                <?= replace_escaped_objects($this_module->language->new->linkedin_company->info, array(
                    '{$link}' => sprintf(
                        "https://bardcanvas.com/autopush-for-linkedin-authorization?start=%s",
                        urlencode(encrypt("company\t{$sp_url}\t{$sp_name}\t{$sp_icon}", $encryption_key))
                    )
                )) ?>
            </div>
            
            <table class="nav_table_inner">
                <? foreach(array("title", "token", "target") as $field): ?>
                    <tr>
                        <th nowrap><?= $this_module->language->new->linkedin_company->fields->{$field}->caption ?></th>
                        <td width="100%">
                            <? if( $field == "token" ): ?>
                                <textarea class="expandible_textarea" data-field="<?= $field ?>"
                                          placeholder="<?= $this_module->language->new->linkedin_profile->fields->{$field}->placeholder ?>"
                                          onblur="rebuild_autopush_endpoints()"></textarea>
                            <? else: ?>
                                <input type="text" data-field="<?= $field ?>"
                                       placeholder="<?= $this_module->language->new->linkedin_profile->fields->{$field}->placeholder ?>"
                                       style="width: 100%" onblur="rebuild_autopush_endpoints()">
                            <? endif; ?>
                        </td>
                    </tr>
                <? endforeach; ?>
            </table>
        </fieldset>
    </template>
    
</div>
