import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseTreeViewDelegate {
    id: control

    property var backgroundColorEnum: UIColorToken.Sidebar_Item_Background
    property real backgroundRadius: 2
    property var textColorEnum: UIColorToken.Sidebar_Item_Text

    implicitWidth: treeView ? treeView.width : 1
    implicitHeight: Math.max(
        contentItem ? contentItem.implicitHeight : 0,
        indicator   ? indicator.implicitHeight   : 0
    ) + topPadding + bottomPadding
    indentation: 8
    leftMargin: 8
    spacing: 6
    topPadding: 4
    bottomPadding: 4

    indicator: Item {
        x: control.leftMargin + control.depth * control.indentation
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: 20
        implicitHeight: 20
        visible: control.hasChildren

        UTLabel {
            anchors.centerIn: parent
            enabled: false
            text: "\u25B6"
            fontEnum: UIFontToken.Caption_Text
            colorEnum: control.textColorEnum
            rotation: control.expanded ? 90 : 0
        }
    }

    contentItem: UTLabel {
        text: control.text
        fontEnum: UIFontToken.Body_Text
        colorEnum: control.textColorEnum
        colorState: control.highlighted ? UIColorState.Selected : UIColorState.Normal
    }

    background: Rectangle {
        color: control.highlighted
            ? UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Selected)
            : control.pressed
                ? UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Pressed)
                : control.hovered
                    ? UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Hovered)
                    : "transparent"
        border.width: control.isDropTarget ? 2 : 0
        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)
        radius: control.backgroundRadius
    }

    UTFocusItem {
        delegateFocused: control.current && control.treeView.activeFocus
        focusRadius: control.backgroundRadius
    }
}
