
function getItemUIColor(widget, colorEnum) {
    var colorState = !widget.enabled ? UIElementData.UIColorState_Disabled :
                      widget.pressed ? UIElementData.UIColorState_Pressed :
                      widget.hovered ? UIElementData.UIColorState_Hovered :
                      widget.activeFocus ? UIElementData.UIColorState_Focused :
                      widget.checked ? UIElementData.UIColorState_Checked : UIElementData.UIColorState_Normal;
    return ThemeManager.getUIColor(colorEnum, colorState);
}

function getPlainUIColor(colorEnum, colorState) {
    return ThemeManager.getUIColor(colorEnum, colorState);
}

function getUIFont(fontSize) {
    return ThemeManager.getUIFont(fontSize)
}