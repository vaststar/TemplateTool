import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts
import QtQml.Models
import UIView 1.0
import UIManager 1.0
import UIComponent 1.0

MenuBar {
    id: root
    property alias controller: controller
    AppMenuBarController{
        id: controller
    }

    Component.onCompleted:{
        console.log("listlength:" + controller.listMenu.length)
        CreateMenus.createMenus(root, controller.listMenu)
        }


    // }

    // Menu{
    //     title: controller.rootMenu.subItems[0].name
    // }
    
    // Menu{
    //     title: controller.rootMenu.subItems[1].name
    // }
    // Instantiator{
    //     model: [1,2,3]
    //     Menu{
    //         id:mm
    //         title: "aaa"
    //         Component.onCompleted: {
    //                         root.addMenu(mm);
    //                 }
    //         }
    // }
    // Instantiator {
    //     model: controller.rootMenu.subItems
    //     Menu{
    //         id: tempMenu
    //         title: qsTr(modelData.name)
    //         Repeater {
    //             id: tempItem
    //             model: modelData.subItems
    //             MenuItem{
    //                 text: modelData.name
    //             }
    //         }
    //         Component.onCompleted: {
    //             root.addMenu(tempMenu);
    //         }
    //     }
    // }
    


     Menu {
        title: qsTr("File")
        Action { text: qsTr("New...") }
        Action { text: qsTr("Open...") }
        Action { text: qsTr("Save") }
        Action { text: qsTr("Save As...") }
        MenuSeparator { }
        Action { text: qsTr("Quit") }
    }
    // Menu {
    //     title: qsTr("Edit")
    //     Action { text: qsTr("Cut") }
    //     Action { text: qsTr("Copy") }
    //     Action { text: qsTr("Paste") }
    // }
    // Menu {
    //     title: qsTr("Settings")
    //     Menu {
    //         title: qsTr("Switch Language")
    //         Action { 
    //             text: qsTr("Switch to Chinese")
    //             onTriggered: {
    //                 controller.switchLanguage(UIManager.LanguageType.CHINESE_SIMPLIFIED)
    //             }
    //         }
    //     }
    // }
    // Menu {
    //     title: qsTr("Help")
    //     Action { text: qsTr("About") }
    // }
}