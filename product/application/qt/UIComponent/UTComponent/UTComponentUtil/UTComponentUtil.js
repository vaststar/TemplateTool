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
    void(UIResourceLoaderManager ? UIResourceLoaderManager.themeRevision : 0);
    const colorState = computeColorState(widget);
    return UIResourceLoaderManager ? UIResourceLoaderManager.getUIColor(colorEnum, colorState) : "red";
}

function getPlainUIColor(colorEnum, colorState) {
    void(UIResourceLoaderManager ? UIResourceLoaderManager.themeRevision : 0);
    return UIResourceLoaderManager ? UIResourceLoaderManager.getUIColor(colorEnum, colorState) : "red";
}

function getUIFont(fontToken) {
    return UIResourceLoaderManager ? UIResourceLoaderManager.getUIFont(fontToken) : Qt.font({ pixelSize: 12 });
}

function getLocalizedString(key) {
    return UIResourceLoaderManager ? UIResourceLoaderManager.getLocalizedString(key) : "No UIResourceLoaderManager";
}

function getImageResourcePath(imageToken) {
    return UIResourceLoaderManager ? UIResourceLoaderManager.getImageResourcePath(imageToken) : "";
}

// Semantic message visuals belong to UT. Unknown / absent semantics use None.
function getMessageIconAssetToken(iconName) {
    switch (iconName) {
    case "info": return UIAssetImageToken.Msg_Info;
    case "warning": return UIAssetImageToken.Msg_Warning;
    case "error": return UIAssetImageToken.Msg_Error;
    case "question": return UIAssetImageToken.Msg_Question;
    case "success": return UIAssetImageToken.Msg_Success;
    default: return UIAssetImageToken.None;
    }
}

function getMessageIconColorToken(iconName) {
    switch (iconName) {
    case "info": return UIColorToken.Message_Info_Icon;
    case "warning": return UIColorToken.Message_Warning_Icon;
    case "error": return UIColorToken.Message_Error_Icon;
    case "question": return UIColorToken.Message_Question_Icon;
    case "success": return UIColorToken.Message_Success_Icon;
    default: return UIColorToken.Content_Text;
    }
}
