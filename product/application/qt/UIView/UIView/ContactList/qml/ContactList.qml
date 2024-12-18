import QtQuick
import QtQuick.Controls
import UIView 1.0

Item{
    id: root
    
    property alias controller: controller

    ContactListViewController{
        id:controller
    }

    Image {
        focus: false
        height:200
        width:200
        source: "qrc:/qt/qml/UIView/picture/112.png"
    }

    Button {
        id: bbb
        text: "contactListButton"
        anchors{
            top: parent.top
            topMargin: 100
            left: parent.left
            leftMargin: 100
        }
        onClicked:{
            console.log("111111111111111")
            controller.buttonClicked()
        }
        background: Rectangle{
            anchors.fill: parent
            border.color: "red"
            border.width: bbb.activeFocus?3:0
        }
    }


    ContactListItemModel{
        id:treeModel
    }

    TreeView {
        id: treeView
        focus: false
        anchors{
            left:parent.left
            leftMargin: 200
            top: parent.top
            topMargin: 10
        }
        width:200
        height:200
        clip: true

        // The model needs to be a QAbstractItemModel
        model: treeModel

        delegate: TreeViewDelegate { implicitWidth : parent.width}
    }

    
    Instantiator{
        model: 3
        delegate: Rectangle {
            width: 100
            height: 100
            color: "lightblue"
            border.color: "black"
            x: index * (width + 10)
        }

    }
}

