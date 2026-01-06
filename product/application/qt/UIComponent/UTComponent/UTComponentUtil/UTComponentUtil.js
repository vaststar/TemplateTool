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
    return UIResourceLoaderManager ? UIResourceLoaderManager.getUIColor(colorEnum, colorState) : {};
}

function getPlainUIColor(colorEnum, colorState) {
    return UIResourceLoaderManager ? UIResourceLoaderManager.getUIColor(colorEnum, colorState) : {};
}

function getUIFont(fontToken) {
    return UIResourceLoaderManager ? UIResourceLoaderManager.getUIFont(fontToken) : {};
}

function getLocalizedString(key) {
    return UIResourceLoaderManager ? UIResourceLoaderManager.getLocalizedString(key) : "No UIResourceLoaderManager";
}

function getImageResourcePath(imageToken) {
    return UIResourceLoaderManager ? UIResourceLoaderManager.getImageResourcePath(imageToken) : {};
}