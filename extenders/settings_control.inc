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

global $config, $settings, $template;

/** @var array $params [name => string, value => string, options => SimpleXMLElement] */
$params = $config->globals["specs_renderer/autopush_control"];

$encryption_key = "bfxFUbgQbQ7QNCmVaK";

$sp_url  = $config->full_root_url;
$sp_name = $settings->get('engine.website_name');
$sp_icon = $settings->get('engine.favicon');
if( empty($sp_icon) ) $sp_icon = $template->get("favicon");
if( empty($sp_icon) ) $sp_icon = "none";
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
                            <i class="autopush-icon twitter"></i>
                            <span class="pseudo_link clipboard-copy" data-clipboard-text="<?= $title ?>"
                                  ><?= $title ?> <i class="fa fa-copy"></i></span>
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
                    $line  = substr($line, 9);
                    $parts = explode(", ", $line);
                    $title = array_shift($parts);
                    ?>
                    <fieldset data-network="discord">
                        <legend>
                            <i class="autopush-icon discord"></i>
                            <span class="pseudo_link clipboard-copy" data-clipboard-text="<?= $title ?>"
                                  ><?= $title ?> <i class="fa fa-copy"></i></span>
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
                elseif( substr($line, 0, 9) == "telegram:" )
                {
                    $line  = substr($line, 10);
                    $parts = explode(", ", $line);
                    $title = array_shift($parts);
                    ?>
                    <fieldset data-network="telegram">
                        <legend>
                            <i class="autopush-icon telegram"></i>
                            <span class="pseudo_link clipboard-copy" data-clipboard-text="<?= $title ?>"
                                  ><?= $title ?> <i class="fa fa-copy"></i></span>
                            <button onclick="remove_autopush_endpoint(this); return false;">
                                <i class="fa fa-trash"></i>
                                <?= $this_module->language->remove ?>
                            </button>
                        </legend>
                        <input type="hidden" data-field="title" value="<?= $title ?>">
                        <table class="nav_table_inner">
                            <? foreach(array("token", "target") as $key => $field): ?>
                                <tr>
                                    <th nowrap><?= $this_module->language->new->telegram->fields->{$field}->caption ?></th>
                                    <td width="100%">
                                        <input type="text" data-field="<?= $field ?>" value="<?= $parts[$key] ?>"
                                               placeholder="<?= $this_module->language->new->telegram->fields->{$field}->placeholder ?>"
                                               style="width: 100%" onblur="rebuild_autopush_endpoints()">
                                    </td>
                                </tr>
                            <? endforeach; ?>
                        </table>
                    </fieldset>
                    <?
                }
                elseif( substr($line, 0, 17) == "linkedin_profile:" )
                {
                    $line  = substr($line, 18);
                    $parts = explode(", ", $line);
                    $title = array_shift($parts);
                    
                    $token_data  = decrypt(trim($parts[0]), $encryption_key);
                    list($token, $expiration) = explode("\t", $token_data);
                    $resource_id = decrypt(trim($parts[1]), $encryption_key);
                    
                    $resource_data = decrypt(trim($parts[1]), $encryption_key);
                    list($user_resource, $company_resource) = explode("\t", $resource_data);
                    ?>
                    <fieldset data-network="linkedin_profile">
                        <legend>
                            <i class="autopush-icon linkedin_profile"></i>
                            <span class="pseudo_link clipboard-copy" data-clipboard-text="<?= $title ?>"
                                  ><?= $title ?> <i class="fa fa-copy"></i></span>
                            <button onclick="remove_autopush_endpoint(this); return false;">
                                <i class="fa fa-trash"></i>
                                <?= $this_module->language->remove ?>
                            </button>
                        </legend>
                        <input type="hidden" data-field="title" value="<?= $title ?>">
                        <table class="nav_table_inner">
                            <? foreach(array("token", "target") as $key => $field): ?>
                                <tr>
                                    <th nowrap><?= $this_module->language->new->linkedin_profile->fields->{$field}->caption ?></th>
                                    <td width="100%">
                                        <? if( $field == "token" ): ?>
                                            <textarea class="expandible_textarea" data-field="<?= $field ?>"
                                                   placeholder="<?= $this_module->language->new->linkedin_profile->fields->{$field}->placeholder ?>"
                                                   style="width: 100%;"
                                                   onblur="rebuild_autopush_endpoints()"><?= $parts[$key] ?></textarea>
                                        <? else: ?>
                                            <input type="text" data-field="<?= $field ?>" value="<?= $parts[$key] ?>"
                                                   placeholder="<?= $this_module->language->new->linkedin_profile->fields->{$field}->placeholder ?>"
                                                   style="width: 100%" onblur="rebuild_autopush_endpoints()">
                                        <? endif; ?>
                                    </td>
                                </tr>
                            <? endforeach;
                            
                            $linkedin_start_string = urlencode(encrypt(
                                "profile\t{$sp_url}\t{$sp_name}\t{$sp_icon}",
                                $encryption_key
                            ));
                            ?>
                            <tr>
                                <th nowrap><?= $this_module->language->new->linkedin_profile->token_expiration ?></th>
                                <td width="100%">
                                    <?
                                    if( ! is_numeric($expiration) )
                                    {
                                        echo replace_escaped_objects(
                                            $this_module->language->linkedin_profile->invalid_token,
                                            array('{$link}' => sprintf(
                                                "https://bardcanvas.com/autopush-for-linkedin-authorization?start=%s",
                                                $linkedin_start_string
                                            ))
                                        );
                                    }
                                    else
                                    {
                                        $exp_date = date("Y-m-d H:i:s", $expiration);
                                        echo $exp_date . " (" . time_remaining_string($exp_date) . ")";
                                    }
                                    ?>
                                </td>
                            </tr>
                            <tr>
                                <th nowrap><?= $this_module->language->new->linkedin_profile->resource_details ?></th>
                                <td width="100%">
                                    <?
                                    if( ! preg_match('/^user:(.+),(.+)$/', $user_resource, $user_matches) )
                                    {
                                        echo replace_escaped_objects(
                                            $this_module->language->linkedin_profile->invalid_resource,
                                            array('{$link}' => sprintf(
                                                "https://bardcanvas.com/autopush-for-linkedin-authorization?start=%s",
                                                $linkedin_start_string
                                            ))
                                        );
                                    }
                                    else
                                    {
                                        echo $user_matches[2];
                                    }
                                    ?>
                                </td>
                            </tr>
                            <tr>
                                <th colspan="2" align="center">
                                    <a href="https://bardcanvas.com/autopush-for-linkedin-authorization?start=<?= $linkedin_start_string ?>"
                                       class="framed_content inlined state_ok" target="_blank">
                                        <i class="fa fa-pencil"></i> <?= $this_module->language->replace ?>
                                    </a>
                                </th>
                            </tr>
                        </table>
                    </fieldset>
                    <?
                }
                elseif( substr($line, 0, 17) == "linkedin_company:" )
                {
                    $line  = substr($line, 18);
                    $parts = explode(", ", $line);
                    $title = array_shift($parts);
                    
                    $token_data  = decrypt(trim($parts[0]), $encryption_key);
                    list($token, $expiration) = explode("\t", $token_data);
                    $resource_id = decrypt(trim($parts[1]), $encryption_key);
                    
                    $resource_data = decrypt(trim($parts[1]), $encryption_key);
                    list($user_resource, $company_resource) = explode("\t", $resource_data);
                    ?>
                    <fieldset data-network="linkedin_company">
                        <legend>
                            <i class="autopush-icon linkedin_company"></i>
                            <span class="pseudo_link clipboard-copy" data-clipboard-text="<?= $title ?>"
                                  ><?= $title ?> <i class="fa fa-copy"></i></span>
                            <button onclick="remove_autopush_endpoint(this); return false;">
                                <i class="fa fa-trash"></i>
                                <?= $this_module->language->remove ?>
                            </button>
                        </legend>
                        <input type="hidden" data-field="title" value="<?= $title ?>">
                        <table class="nav_table_inner">
                            <? foreach(array("token", "target") as $key => $field): ?>
                                <tr>
                                    <th nowrap><?= $this_module->language->new->linkedin_company->fields->{$field}->caption ?></th>
                                    <td width="100%">
                                        <? if( $field == "token" ): ?>
                                            <textarea class="expandible_textarea" data-field="<?= $field ?>"
                                                   placeholder="<?= $this_module->language->new->linkedin_profile->fields->{$field}->placeholder ?>"
                                                   style="width: 100%;"
                                                   onblur="rebuild_autopush_endpoints()"><?= $parts[$key] ?></textarea>
                                        <? else: ?>
                                            <input type="text" data-field="<?= $field ?>" value="<?= $parts[$key] ?>"
                                                   placeholder="<?= $this_module->language->new->linkedin_profile->fields->{$field}->placeholder ?>"
                                                   style="width: 100%" onblur="rebuild_autopush_endpoints()">
                                        <? endif; ?>
                                    </td>
                                </tr>
                            <? endforeach;
                            
                            $linkedin_start_string = urlencode(encrypt(
                                "company\t{$sp_url}\t{$sp_name}\t{$sp_icon}",
                                $encryption_key
                            ));
                            ?>
                            <tr>
                                <th nowrap><?= $this_module->language->new->linkedin_profile->token_expiration ?></th>
                                <td width="100%">
                                    <?
                                    if( ! is_numeric($expiration) )
                                    {
                                        echo replace_escaped_objects(
                                            $this_module->language->linkedin_profile->invalid_token,
                                            array('{$link}' => sprintf(
                                                "https://bardcanvas.com/autopush-for-linkedin-authorization?start=%s",
                                                $linkedin_start_string
                                            ))
                                        );
                                    }
                                    else
                                    {
                                        $exp_date = date("Y-m-d H:i:s", $expiration);
                                        echo $exp_date . " (" . time_remaining_string($exp_date) . ")";
                                    }
                                    ?>
                                </td>
                            </tr>
                            <tr>
                                <th nowrap><?= $this_module->language->new->linkedin_profile->resource_details ?></th>
                                <td width="100%">
                                    <?
                                    preg_match('/^user:(.+),(.+)$/', $user_resource, $user_matches);
                                    
                                    if( ! preg_match('/^company:(.+),(.+)$/', $company_resource, $comp_matches) )
                                    {
                                        echo replace_escaped_objects(
                                            $this_module->language->linkedin_profile->invalid_resource,
                                            array('{$link}' => sprintf(
                                                "https://bardcanvas.com/autopush-for-linkedin-authorization?start=%s",
                                                $linkedin_start_string
                                            ))
                                        );
                                    }
                                    else
                                    {
                                        # echo "$user_resource - $company_resource<br>";
                                        if( ! empty($user_matches[2]) )
                                            echo "{$user_matches[2]} <i class='fa fa-chevron-right'></i> ";
                                        
                                        echo "<a href='https://www.linkedin.com/company/{$comp_matches[1]}/' "
                                        .    "   target='_blank'>{$comp_matches[2]}</a>";
                                    }
                                    ?>
                                </td>
                            </tr>
                            <tr>
                                <th colspan="2" align="center">
                                    <a href="https://bardcanvas.com/autopush-for-linkedin-authorization?start=<?= $linkedin_start_string ?>"
                                       class="framed_content inlined state_ok" target="_blank">
                                        <i class="fa fa-pencil"></i> <?= $this_module->language->replace ?>
                                    </a>
                                </th>
                            </tr>
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
