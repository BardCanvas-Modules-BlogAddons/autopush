
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
