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
    
    #autopush_endpoints_container .target fieldset.new { box-shadow: 0 0 5px green; }
</style>

<script type="text/javascript">
    
    function add_autopush_template(key)
    {
        if( key === '' ) return;
        
        var html    = $('#autopush_endpoint_templates').find(sprintf('template.%s', key)).html();
        var $target = $('#autopush_endpoints_container').find('.target');
        $target.append(html);
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
                <?= $this_module->language->new->twitter->legend ?>
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
</div>
