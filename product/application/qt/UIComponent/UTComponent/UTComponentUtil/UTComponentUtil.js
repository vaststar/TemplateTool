
function computeColorState(widget) {
    if (!widget.enabled) return UIColorState.Disabled;
    if (widget.pressed) return UIColorState.Pressed;
    if (widget.hovered) return UIColorState.Hovered;
    if (widget.activeFocus) return UIColorState.Focused;
    if (widget.selected) return UIColorState.Selected;
    if (widget.checked) return UIColorState.Checked;
    return UIColorState.Normal;
}

function getItemUIColor(widget, colorEnum) {
    const colorState = computeColorState(widget);
    return ThemeManager ? ThemeManager.getUIColor(colorEnum, colorState) : {};
}

function getPlainUIColor(colorEnum, colorState) {
    return ThemeManager ? ThemeManager.getUIColor(colorEnum, colorState) : {};
}

function getUIFont(fontSize) {
    return ThemeManager ? ThemeManager.getUIFont(fontSize) : {};
}

function getLocalizedString(key) {
    console.log("6666666666")
    return ThemeManager ? ThemeManager.getLocalizedString(key) : "No ThemeManager";
}