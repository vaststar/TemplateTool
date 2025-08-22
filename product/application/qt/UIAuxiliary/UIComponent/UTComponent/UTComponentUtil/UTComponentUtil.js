
function getItemUIColor(widget, colorEnum) {
    var colorState = !widget.enabled ? UIColorState.Disabled :
                      widget.pressed ? UIColorState.Pressed :
                      widget.hovered ? UIColorState.Hovered :
                      widget.activeFocus ? UIColorState.Focused :
                      widget.checked ? UIColorState.Checked : UIColorState.Normal;
    return ThemeManager ? ThemeManager.getUIColor(colorEnum, UIColorState.Normal) : {};
}

function getPlainUIColor(colorEnum, colorState) {
    return ThemeManager ? ThemeManager.getUIColor(colorEnum, colorState) : {};
}

function getUIFont(fontSize) {
    return ThemeManager ? ThemeManager.getUIFont(fontSize) : {};
}

function getLocalizedString(key) {
    return ThemeManager ? ThemeManager.getLocalizedString(key) : "No ThemeManager";
}