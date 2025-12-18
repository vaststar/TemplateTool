import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0

ToolBar {
    id: root
    property MainWindowFootBarController controller: MainWindowFootBarController{}
    RowLayout {
        anchors.fill: parent
        ToolButton {
            text: qsTr("‹")
            onClicked: stack.pop()
        }
        Label {
            text: qsTr(root.controller.footerName)
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
    Component.onCompleted:{
        ControllerInitializer.initializeController(root.controller)
    }
}
