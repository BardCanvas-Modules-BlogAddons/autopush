<?php
/**
 * Settings field type extender
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

global $config, $settings;

/** @var array $params [name => string, value => string, options => SimpleXMLElement] */
$params = $config->globals["specs_renderer/autopush_control"];
?>

<div id="autopush_endpoints_container">
    
    <textarea class="target_value" wrap="off" name="names[<?= $params["name"] ?>]"
              style="display: none"><?= $params["value"] ?></textarea>
    
    <div class="target">
        <?
        if( ! empty($params["value"]) )
        {
            foreach(explode("\n", $params["value"]) as $line)
            {
                $line = trim($line);
                if( empty($line) ) continue;
                
                if( substr($line, 0, 8) == "twitter:" )
                {
                    $line  = substr($line, 9);
                    $parts = explode(", ", $line);
                    $title = array_shift($parts);
                    ?>
                    <fieldset data-network="twitter">
                        <legend>
                            <i class="autopush-icon twitter"></i> <?= $title ?>
                            <button onclick="remove_autopush_endpoint(this); return false;">
                                <i class="fa fa-trash"></i>
                                <?= $this_module->language->remove ?>
                            </button>
                        </legend>
                        <input type="hidden" data-field="title" value="<?= $title ?>">
                        <table class="nav_table_inner">
                            <? foreach(array("consumer_key", "consumer_secret", "token", "token_secret") as $key => $field): ?>
                                <tr>
                                    <th nowrap><?= $this_module->language->new->twitter->fields->{$field}->caption ?></th>
                                    <td width="100%">
                                        <input type="text" data-field="<?= $field ?>" value="<?= $parts[$key] ?>"
                                               placeholder="<?= $this_module->language->new->twitter->fields->{$field}->placeholder ?>"
                                               style="width: 100%" onblur="rebuild_autopush_endpoints()">
                                    </td>
                                </tr>
                            <? endforeach; ?>
                        </table>
                    </fieldset>
                    <?
                }
                elseif( substr($line, 0, 8) == "discord:" )
                {
                    $line = substr($line, 9);
                    $parts = explode(", ", $line);
                    $title = array_shift($parts);
                    ?>
                    <fieldset data-network="discord">
                        <legend>
                            <i class="autopush-icon discord"></i> <?= $title ?>
                            <button onclick="remove_autopush_endpoint(this); return false;">
                                <i class="fa fa-trash"></i>
                                <?= $this_module->language->remove ?>
                            </button>
                        </legend>
                        <input type="hidden" data-field="title" value="<?= $title ?>">
                        <table class="nav_table_inner">
                            <? foreach(array("webhook") as $key => $field): ?>
                                <tr>
                                    <th nowrap><?= $this_module->language->new->discord->fields->{$field}->caption ?></th>
                                    <td width="100%">
                                        <input type="text" data-field="<?= $field ?>" value="<?= $parts[$key] ?>"
                                               placeholder="<?= $this_module->language->new->discord->fields->{$field}->placeholder ?>"
                                               style="width: 100%" onblur="rebuild_autopush_endpoints()">
                                    </td>
                                </tr>
                            <? endforeach; ?>
                        </table>
                    </fieldset>
                    <?
                }
            }
        }
        ?>
    </div>
    
    <div>
        <select id="add_endpoint_selector" onchange="add_autopush_template( this.options[this.selectedIndex].value )"
                style="font-size: 12pt; width: auto;">
            <option value="" disabled selected><?= $this_module->language->add_selector->default ?></option>
            <? foreach($this_module->language->add_selector->option as $option): ?>
                <option value="<?= $option["value"] ?>"><?= $option ?></option>
            <? endforeach ?>
        </select>
    </div>
    
</div>
