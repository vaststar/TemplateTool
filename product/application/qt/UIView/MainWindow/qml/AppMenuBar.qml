import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts
import UIView 1.0
import UIManager 1.0

MenuBar {
    id: root
    property alias controller: controller
    AppMenuBarController{
        id: controller
    }
     Menu {
        title: qsTr("File")
        Action { text: qsTr("New...") }
        Action { text: qsTr("Open...") }
        Action { text: qsTr("Save") }
        Action { text: qsTr("Save As...") }
        MenuSeparator { }
        Action { text: qsTr("Quit") }
    }
    Menu {
        title: qsTr("Edit")
        Action { text: qsTr("Cut") }
        Action { text: qsTr("Copy") }
        Action { text: qsTr("Paste") }
    }
    Menu {
        title: qsTr("Settings")
        Menu {
            title: qsTr("Switch Language")
            Action { 
                text: qsTr("Switch to Chinese")
                onTriggered: {
                    controller.switchLanguage(UIManager.LanguageType.CHINESE_SIMPLIFIED)
                }
            }
        }
    }
    Menu {
        title: qsTr("Help")
        Action { text: qsTr("About") }
    }
}