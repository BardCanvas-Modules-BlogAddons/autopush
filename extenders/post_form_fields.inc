<?php
/**
 * Autopush form fields
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

?>
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
