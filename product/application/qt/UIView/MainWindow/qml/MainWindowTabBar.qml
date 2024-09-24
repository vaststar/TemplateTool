import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
TabBar {
    id: bar
    width: root.width/8
    TabButton {
        id: firstBtn
        text: qsTr("Home")
        width: root.width/8
        height: root.height/3
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
        }
        // anchors.horizontalCenter: parent.horizontalCenter
        // anchors.top: parent.top
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