import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item{
    id: mainWindowContent
    required property MainWindowController controller

    // Sidebar with resizable width
    MainWindowSideBar{
        id: navigationBar
        width: 168
        height: parent.height
        
        property int minWidth: 140
        property int maxWidth: 280
    }
    
    // Resize handle for sidebar
    Rectangle {
        id: resizeHandle
        width: 4
        height: parent.height
        x: navigationBar.width - 2
        color: resizeArea.containsMouse || resizeArea.pressed ? "#1976D2" : "transparent"
        
        MouseArea {
            id: resizeArea
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.SplitHCursor
            
            property int startX: 0
            property int startWidth: 0
            
            onPressed: (mouse) => {
                startX = mouse.x + resizeHandle.x
                startWidth = navigationBar.width
            }
            
            onPositionChanged: (mouse) => {
                if (pressed) {
                    let newWidth = startWidth + (mouse.x + resizeHandle.x - startX)
                    navigationBar.width = Math.max(navigationBar.minWidth, 
                        Math.min(navigationBar.maxWidth, newWidth))
                }
            }
        }
    }

    ContactList{
        id: frame
        width : 400
        height: 200
        anchors{
            top: parent.top
            left: navigationBar.right
            leftMargin: 4
        }
    }

    UTQRCode {
        text: "这是测试这是测试这是测试，我靠"
        displaySize: 200
        errorLevel: UTQRCodeLevel.High
        darkColor: "#FF5722"
        lightColor: "#FFF3E0"
        anchors {
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