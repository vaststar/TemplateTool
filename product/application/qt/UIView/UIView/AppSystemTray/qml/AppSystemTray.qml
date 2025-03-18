import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import UIView 1.0

SystemTrayIcon {
    id: root
    property var controller: AppSystemTrayController{}

    visible: true
    icon.source: "qrc:/qt/qml/UIView/picture/112.png"

     menu: Menu {
        MenuItem {
            text: qsTr("Quit")
            onTriggered: Qt.quit()
        }
    }
}