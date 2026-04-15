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
 *       ColumnLayout {
 *           // content here
 *       }
 *   }
 */
BaseGroupBox {
    id: control

    property var titleColorEnum: UIColorToken.Content_Section_Title
    property var backgroundColorEnum: UIColorToken.Content_Section_Background
    property var borderColorEnum: UIColorToken.Content_Section_Border
    property int borderRadius: 4

    font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)

    label: UTText {
        x: control.leftPadding
        text: control.title
        fontEnum: UIFontToken.Body_Text_Medium
        colorEnum: control.titleColorEnum
    }

    background: Rectangle {
        y: control.topPadding - control.bottomPadding
        width: parent.width
        height: parent.height - control.topPadding + control.bottomPadding
        color: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
        border.color: UTComponentUtil.getPlainUIColor(control.borderColorEnum, UIColorState.Normal)
        border.width: 1
        radius: control.borderRadius
    }
}
