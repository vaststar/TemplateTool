import QtQuick
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root

    required property ContactsPageController controller

    // Panel background – slightly different from content area
    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)
    }

    // Header
    Rectangle {
        id: header
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: 40
        color: "transparent"

        Text {
            anchors {
                left: parent.left
                leftMargin: 12
                verticalCenter: parent.verticalCenter
            }
            text: "联系人列表"
            font.pixelSize: 14
            font.bold: true
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)
        }
    }

    // Tree container with clip boundary for focus ring
    Item {
        id: treeContainer
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }
        clip: true

        TreeView {
            id: treeView
            anchors.fill: parent
            anchors.margins: 4
            clip: false
            activeFocusOnTab: true

            model: controller.orgTreeModel
            selectionModel: ItemSelectionModel {}

            Keys.onTabPressed: function(e) {
                e.accepted = true;
                const next = treeView.nextItemInFocusChain(true);
                if (next && next !== treeView) next.forceActiveFocus(Qt.TabFocusReason);
            }

            Keys.onBacktabPressed: function(e) {
                e.accepted = true;
                const prev = treeView.nextItemInFocusChain(false);
                if (prev && prev !== treeView) prev.forceActiveFocus(Qt.BacktabFocusReason);
            }

            onActiveFocusChanged: {
                if (activeFocus) Qt.callLater(function() {
                    if (treeView.currentRow < 0 && treeView.model
                            && treeView.model.rowCount(treeView.rootIndex) > 0) {
                        const idx = treeView.index(0, 0);
                        treeView.selectionModel.setCurrentIndex(
                            idx, ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Rows);
                    }
                });
            }

            delegate: Item {
                implicitWidth: padding + label.x + label.implicitWidth + padding
                implicitHeight: label.implicitHeight * 1.8
                z: (current && treeView.activeFocus) ? 1 : 0

                readonly property real indentation: 8
                readonly property real padding: 8

                // Assigned to by TreeView:
                required property TreeView treeView
                required property bool isTreeNode
                required property bool expanded
                required property bool hasChildren
                required property int depth
                required property int row
                required property int column
                required property bool current

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
                    color: row === treeView.currentRow
                        ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
                        : hoverHandler.hovered
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
                            : "transparent"
                }

                Label {
                    id: indicator
                    x: padding + (depth * indentation)
                    anchors.verticalCenter: parent.verticalCenter
                    visible: isTreeNode && hasChildren
                    text: "▶"
                    color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)

                    TapHandler {
                        onSingleTapped: {
                            const idx = treeView.index(row, column);
                            treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.NoUpdate);
                            treeView.toggleExpanded(row);
                        }
                    }
                }

                Label {
                    id: label
                    x: padding + (isTreeNode ? (depth + 1) * indentation : 0)
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - padding - x
                    text: model.displayName
                    color: row === treeView.currentRow
                        ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Selected)
                        : UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)
                }

                UTFocusItem {
                    externallyShown: current && treeView.activeFocus
                }
            }
        }
    }
}
