import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts
import QtQml.Models
import UIView 1.0
import UIManager 1.0
import UTComponent 1.0

MenuBar {
    id: root
    property alias controller: controller
    AppMenuBarController{
        id: controller
    }

    Component.onCompleted:{
        controller.controllerInitialized.connect(createMenuItems)
        }

    function createMenuItems(){
        console.log("listlength:" + controller.listMenu.length)
        CreateMenus.createMenu(root, controller.rootMenu)
// CreateMenus.createMenu(root, controller.rootMenu)
    }
}