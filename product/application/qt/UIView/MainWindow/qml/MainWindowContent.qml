import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UIComponent 1.0
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
        // controller.controllerInitialized.connect(frame.controller.initializeController)
        // frame.controller.initializeController(controller.getAppContext())
        controller.onContactListLoaded(frame.controller)
    }

    Text {
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
        text: "testDialogShowButton"
        anchors{
            top: parent.top
            topMargin: 300
            left: parent.left
            leftMargin: 100
        }
        onClicked:{
            dialog.open()
        }
    }

    UTDialog{
        id: dialog
    }
}