import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts
TabBar {
    id: bar
    width: firstBtn.width
    TabButton {
        id: firstBtn
        text: qsTr("Home")
        width: root.width/8
        height: root.height/3
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
    }
    TabButton {
        id: secondBtn
        text: qsTr("Discover")
        width: root.width/8
        height: root.height/3
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: firstBtn.bottom
    }
    TabButton {
        id: thirdBtn
        text: qsTr("Activity")
        width: root.width/8
        height: root.height/3
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: secondBtn.bottom
    }
}