import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts
import QtQml.Models
import QtQml
import UIView 1.0
import UTComponent 1.0

MenuBar {
    id: root
    property var controller: MainWindowMenuBarController{}

    Component.onCompleted:{
        controller.controllerInitialized.connect(createMenuItems)
        }

    function createMenuItems(){
        console.log("listlength:" + controller.listMenu.length)
        for (let index = 0; index < controller.rootMenu.subItems.length; index++) {
            CreateMenus.createMenu(root, controller.rootMenu.subItems[index])
        }
    }
}