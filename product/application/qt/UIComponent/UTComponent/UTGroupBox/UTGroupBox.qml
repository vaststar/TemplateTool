import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTGroupBox - Themed section container with title and rounded border.
 *
 * Usage:
 *   UTGroupBox {
 *       title: qsTr("Settings")
 *
 *       contentItem: ColumnLayout {
 *           // content here
 *       }
 *   }
 */
BaseGroupBox {
    id: control

    property var titleFontEnum: UIFontToken.Section_Title
    property var titleColorEnum: UIColorToken.Content_Section_Title
    property var backgroundColorEnum: UIColorToken.Content_Section_Background
    property var borderColorEnum: UIColorToken.Content_Section_Border

    font: UTComponentUtil.getUIFont(control.titleFontEnum)
    titleColor: UTComponentUtil.getPlainUIColor(control.titleColorEnum, UIColorState.Normal)
    backgroundColor: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
    borderColor: UTComponentUtil.getPlainUIColor(control.borderColorEnum, UIColorState.Normal)
}
