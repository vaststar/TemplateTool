import QtQuick
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0

Item{
    id: root
    
    property var controller: ContactListViewController{}

    Image {
        height:200
        width:200
        source: "qrc:/qt/qml/UIView/picture/112.png"
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

        // The model needs to be a QAbstractItemModel
        model: treeModel
        selectionModel: ItemSelectionModel {}

        delegate: Item {
            implicitWidth: padding + label.x + label.implicitWidth + padding
            implicitHeight: label.implicitHeight * 1.5
            focus: treeView.currentRow === row && treeView.activeFocus

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

            // Rotate indicator when expanded by the user
            // (requires TreeView to have a selectionModel)
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
                        let index = treeView.index(row, column)
                        treeView.selectionModel.setCurrentIndex(index, ItemSelectionModel.NoUpdate)
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
                z: 10
            }
        }
    }

    
    // Instantiator{
    //     model: 3
    //     delegate: Rectangle {
    //         width: 100
    //         height: 100
    //         color: "black"
    //         border.color: "black"
    //         x: index * (width + 10)
    //     }

    // }
}

