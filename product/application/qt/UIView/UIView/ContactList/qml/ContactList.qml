import QtQuick
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0

Item{
    id: root
    
    property alias controller: controller

    ContactListViewController{
        id:controller
    }

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

