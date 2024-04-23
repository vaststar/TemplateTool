import QtQuick
import QtQuick.Controls
import UIView 1.0

Item{
    id: root
    
    property alias controller: controller

    ContactListViewController{
        id:controller
    }
Datas{
    id: dat
}
property var ii:"555"


function getAge()
{
    console.log("bbbbbbbbbbbbbbbbbbb")
    // console.log("aaa", root.dat.getAge())
    return "aaa"//data.getAge()
}
Image {
    height:200
    width:200
    source: "qrc:/qt/qml/UIView/picture/112.png"
}
Label {
    id:lab
    text: dat.m_age
    font.pixelSize: 22
    font.italic: true
    Component.onCompleted:{
        console.log("done")
        // text = root.dat.getData()
    }
}

// Button{
//     id:butt
//     text:"5555555555"
//     height: 200
//     width: 200
//     x: 200
//     onClicked: {
//         console.log("Button clicked")
//         lab.text = dat.getData()
//     }

// }

ContactListItemModel{id:treeModel}
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
}

