import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    property ToolsPageController controller

    width: 200

    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)
    }

    // Right border
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Border, UIColorState.Normal)
    }

    // Tree container
    UTTreeView {
        id: treeContainer
        anchors.fill: parent
        model: controller ? controller.treeModel : null
        expandAll: true

        onItemActivated: function(idx) {
            var nodeId = treeView.model.data(idx, Qt.UserRole + 1);
            if (nodeId) controller.selectNode(nodeId);
        }

            delegate: Item {
                id: navItem
                implicitWidth: treeView.width
                implicitHeight: 32
                z: (current && treeView.activeFocus) ? 1 : 0

                required property TreeView treeView
                required property bool isTreeNode
                required property bool expanded
                required property int hasChildren
                required property int depth
                required property int row
                required property int column
                required property bool current
                required property string nodeId
                required property string title
                required property int panelType

                property Animation indicatorAnimation: NumberAnimation {
                    target: indicator
                    property: "rotation"
                    from: expanded ? 0 : 90
                    to: expanded ? 90 : 0
                    duration: 100
                    easing.type: Easing.OutQuart
                }

                TableView.onPooled: indicatorAnimation.complete()
                TableView.onReused: if (current) indicatorAnimation.start()
                onExpandedChanged: indicator.rotation = expanded ? 90 : 0

                HoverHandler { id: hoverHandler }

                Rectangle {
                    id: background
                    anchors.fill: parent
                    color: controller && controller.currentNodeId === navItem.nodeId
                        ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
                        : hoverHandler.hovered
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
                            : "transparent"
                }

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 8 + navItem.depth * 16
                    spacing: 4

                    // Expand indicator
                    UTLabel {
                        id: indicator
                        visible: navItem.hasChildren
                        text: "▶"
                        fontEnum: UIFontToken.Caption_Text
                        colorEnum: UIColorToken.Sidebar_Item_Text
                        rotation: navItem.expanded ? 90 : 0
                    }

                    // Placeholder when no children
                    Item {
                        visible: !navItem.hasChildren
                        width: 12
                        height: 12
                    }

                    UTText {
                        text: navItem.title
                        fontEnum: UIFontToken.Body_Text
                        colorEnum: UIColorToken.Sidebar_Item_Text
                        colorState: controller && controller.currentNodeId === navItem.nodeId
                            ? UIColorState.Selected
                            : UIColorState.Normal
                    }
                }

                TapHandler {
                    onSingleTapped: {
                        const idx = treeView.index(navItem.row, navItem.column);
                        treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.NoUpdate);
                        if (navItem.hasChildren) {
                            treeView.toggleExpanded(navItem.row)
                        }
                        controller.selectNode(navItem.nodeId)
                    }
                }

                UTFocusItem {
                    externallyShown: current && treeContainer.treeView.activeFocus
                }
            }
    }
}
