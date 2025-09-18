import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0

ToolBar {
    property alias controller: footBarController
    MainWindowFootBarController {
        id: footBarController
    }
    RowLayout {
        anchors.fill: parent
        ToolButton {
            text: qsTr("‹")
            onClicked: stack.pop()
        }
        Label {
            text: qsTr(controller.footerName)
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
