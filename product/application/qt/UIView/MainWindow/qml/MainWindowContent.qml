import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item{
    id: mainWindowContent
    required property MainWindowController controller

    MainWindowSideBar{
        id: navigationBar
        width: parent.width/16
        height: parent.height
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

    UTQRCode {
    text: "这是测试这是测试这是测试，我靠"
    displaySize: 200
    errorLevel: QRCodeGenerator.High
    darkColor: "#FF5722"
    lightColor: "#FFF3E0"
    anchors{
            bottom: parent.bottom
            right: parent.right
        }
}
    
    Component.onCompleted:{
        if (!controller){
            console.log("MainWindowContent controller is null")
            return
        }
        controller.initController(navigationBar.controller)
        controller.initController(frame.controller)
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

    
    UTButton {
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
    }

    UTButton {
        id: butn1
        text: qsTr("IopenCamera")
        anchors{
            top: parent.top
            topMargin: 350
            left: parent.left
            leftMargin: 50
        }
        onClicked:{
            controller.openCamera();
        }
    }
    UTButton {
        id: butn2
        text:  qsTr(UTComponentUtil.getLocalizedString(UILocalizedStringToken.OKLabel)) //qsTr("button2")
        anchors{
            top: parent.top
            topMargin: 300
            left: parent.left
            leftMargin: 400
        }
        onClicked:{
            controller.testFunc()
        }
    }
    UTButton {
        id: packLogsBtn
        text: qsTr("Pack Logs")
        anchors{
            top: parent.top
            topMargin: 400
            left: parent.left
            leftMargin: 50
        }
        onClicked:{
            controller.packLogs()
        }
    }
    UTDialog{
        id: dialog
    }
}