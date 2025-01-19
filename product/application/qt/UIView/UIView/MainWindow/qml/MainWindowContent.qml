import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIDataStruct 1.0
import UIManager 1.0

Item{
    required property var controller

    MainWindowTabBar{
        id: navigationBar
    }
    ContactList{
        id: frame
        width : 400
        height: 200
        anchors{
            top: parent.top
            left: navigationBar.right
        }
    }
    
    Component.onCompleted:{
        controller.onContactListLoaded(frame.controller)
    }

    Text {
        focus: false
        text: controller.mControllerName + "test"
        font.family: "Helvetica"
        font.pointSize: 12
        color: "red"
        anchors{
            top: parent.top
            topMargin: 200
            left: parent.left
            leftMargin: 200
        }
    }

    
    Button {
        id: bbb
        focus: true
        text: "testDialogShowButton"
        anchors{
            top: parent.top
            topMargin: 300
            left: parent.left
            leftMargin: 50
        }
        onClicked:{
            dialog.open()
        }
        background: Rectangle{
            anchors.fill: parent
            border.color: "red"
            border.width: bbb.activeFocus?3:0
        }
    }

    Button {
        id: butn1
        text: qsTr("openCamera")
        anchors{
            top: parent.top
            topMargin: 300
            left: parent.left
            leftMargin: 200
        }
        background: Rectangle{
            anchors.fill: parent
            border.color: "red"
            border.width: butn1.activeFocus?3:0
        }
        onClicked:{
            controller.openCamera();
        }
    }
    UTButton {
        id: butn2
        text: qsTr("button2")
        width: 300
        anchors{
            top: parent.top
            topMargin: 300
            left: parent.left
            leftMargin: 400
        }
        // background: Rectangle{
        //     anchors.fill: parent
        //     border.color: "red"
        //     border.width: butn2.activeFocus?3:0
        // }
        onClicked:{
            console.log("ttt", UIManager.BBB)
        }
    }
    UTDialog{
        id: dialog
    }
}