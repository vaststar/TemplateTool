import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts
import UIViews 1.0
import mainUI 1.0
// import qt.windowController 1.0

// import QtWebEngine

ApplicationWindow
{
    MainWindowController{
        id:controller
    }
    // property var controller //: MainWindowController
    visible: true
    width: 758 
    height: 576
    title: qsTr("Minimal Qml")
    color: "steelblue"
     header : ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: qsTr("‹")
                onClicked: stack.pop()
            }
            Label {
                text: "Header"
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            ToolButton {
                text: qsTr("⋮")
                onClicked: menu.open()
            }
        }
    }
    menuBar : ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: qsTr("‹")
                onClicked: stack.pop()
            }
            Label {
                text: "ManuBar"
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            ToolButton {
                text: qsTr("⋮")
                onClicked: menu.open()
            }
        }
    }
     footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: qsTr("‹")
                onClicked: stack.pop()
            }
            Label {
                text: "Footer"
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            ToolButton {
                text: qsTr("⋮")
                onClicked: menu.open()
            }
        }
    }
StackView {
        anchors.fill: parent
        initialItem: Rectangle {
            width: 400
            height: 200
            color: "salmon"
        }
    }
Datas{id:dd}
    ContactList{
        id: frame
        width : 400
        height: 200
    }

        Text {
    text: controller.mControllerName
    font.family: "Helvetica"
    font.pointSize: 12
    color: "red"
    
}
ContactList{
    id: contactList
    Component.onCompleted:{
        controller.registerController(contactList.controller)
    }
}
    // WebEngineView{
    //     anchors.fill:parent
    //     url:"https://www.baidu.com"
    // }
}