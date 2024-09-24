import Qt.labs.platform

SystemTrayIcon {
    visible: true
    icon.source: "qrc:/qt/qml/UIView/picture/112.png"

     menu: Menu {
        MenuItem {
            text: qsTr("Quit")
            onTriggered: Qt.quit()
        }
    }
}