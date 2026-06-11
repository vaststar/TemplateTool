import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseTreeViewDelegate {
    id: control

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

    indicator: UTLabel {
        x: control.leftMargin + control.depth * control.indentation
        anchors.verticalCenter: parent.verticalCenter
        text: "\u25B6"
        fontEnum: UIFontToken.Caption_Text
        colorEnum: UIColorToken.Sidebar_Item_Text
        rotation: control.expanded ? 90 : 0
        visible: control.hasChildren
    }

    contentItem: UTLabel {
        text: control.text
        fontEnum: UIFontToken.Body_Text
        colorEnum: UIColorToken.Sidebar_Item_Text
        colorState: control.highlighted ? UIColorState.Selected : UIColorState.Normal
    }

    background: Rectangle {
        color: control.highlighted
            ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
            : control.hovered
                ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
                : "transparent"
        border.width: control.isDropTarget ? 2 : 0
        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)
        radius: 2
    }

    UTFocusItem {
        delegateFocused: control.current && control.treeView.activeFocus
    }
}
