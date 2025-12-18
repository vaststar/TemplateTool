import QtQuick
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item{
    id: root
    
    property ContactListViewController controller:  ContactListViewController{}
    Component.onCompleted:{
        ControllerInitializer.initializeController(root.controller)
    }

    Image {
        height:200
        width:200
        // source: "qrc:/qt/qml/UIView/picture/112.png"
        source: UTComponentUtil.getImageResourcePath(UIAssetImageToken.Logo)//"qrc:/images/logo"
    }

    UTButton {
        focus: true
        id: bbb
        text: "contactListButton"
        anchors{
            top: parent.top
            topMargin: 100
            left: parent.left
            leftMargin: 100
        }
        onClicked:{
            console.log("test")
            controller.buttonClicked()
        }
    }

    ContactListItemModel{
        id:treeModel
    }

    TreeView {
        id: treeView
        anchors{
            left:parent.left
            leftMargin: 300
            top: parent.top
            topMargin: 10
        }
        width:200
        height:200
        clip: false
        activeFocusOnTab: true

        // The model needs to be a QAbstractItemModel
        model: treeModel
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
                if (treeView.currentRow < 0 && treeView.model && treeView.model.rowCount(treeView.rootIndex) > 0) {
                    const idx = treeView.index(0, 0);
                    treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Rows);
                }
            });
        }

        delegate: Item {
            implicitWidth: padding + label.x + label.implicitWidth + padding
            implicitHeight: label.implicitHeight * 1.5

            readonly property real indentation: 20
            readonly property real padding: 5

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

            Rectangle {
                id: background
                anchors.fill: parent
                color: row === treeView.currentRow ? palette.highlight : "white"
                opacity: (treeView.alternatingRows && row % 2 !== 0) ? 0.8 : 0.5
            }


            Label {
                id: indicator
                x: padding + (depth * indentation)
                anchors.verticalCenter: parent.verticalCenter
                visible: isTreeNode && hasChildren
                text: "▶"

                TapHandler {
                    onSingleTapped: {
                        const idx = treeView.index(row, column);
                        treeView.selectionModel.setCurrentIndex(idx, ItemSelectionModel.NoUpdate);
                        treeView.toggleExpanded(row)
                    }
                }
            }

            Label {
                id: label
                x: padding + (isTreeNode ? (depth + 1) * indentation : 0)
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - padding - x
                text: model.display
                
            }
            UTFocusItem{
                externallyShown: current && treeView.activeFocus
            }
        }
    }
}

