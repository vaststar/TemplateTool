import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseTreeViewDelegate {
    id: control

    property var backgroundColorEnum: UIColorToken.Sidebar_Item_Background
    property var textColorEnum: UIColorToken.Sidebar_Item_Text
    property var fontEnum: UIFontToken.Body_Text
    property var indicatorFontEnum: UIFontToken.Caption_Text

    textFont: UTComponentUtil.getUIFont(control.fontEnum)
    indicatorFont: UTComponentUtil.getUIFont(control.indicatorFontEnum)
    textColor: UTComponentUtil.getPlainUIColor(
                   control.textColorEnum,
                   control.highlighted ? UIColorState.Selected : UIColorState.Normal)
    indicatorColor: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
    backgroundColor: control.highlighted
        ? UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Selected)
        : control.pressed
            ? UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Pressed)
            : control.hovered
                ? UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Hovered)
                : "transparent"
    borderColor: UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)

    dragPreviewBackgroundColor: UTComponentUtil.getPlainUIColor(UIColorToken.Tooltip_Background, UIColorState.Normal)
    dragPreviewTextColor: UTComponentUtil.getPlainUIColor(UIColorToken.Tooltip_Text, UIColorState.Normal)
    dragPreviewOpacity: 0.85
    dragPreviewRadius: 4

    UTFocusItem {
        delegateFocused: control.current && control.treeView.activeFocus
        focusRadius: control.backgroundRadius
    }
}
