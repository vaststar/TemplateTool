import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts
import QtQml.Models
import QtQml
import UIView 1.0
import UTComponent 1.0

MenuBar {
    id: root
    property MainWindowMenuBarController controller: MainWindowMenuBarController{}

    Connections {
        target: controller
        onControllerInitialized: createMenuItems()
    }
    
    function createMenuItems(){
        console.log("listlength:" + controller.listMenu.length)
        for (let index = 0; index < controller.rootMenu.subItems.length; index++) {
            CreateMenus.createMenu(root, controller.rootMenu.subItems[index])
        }
    }
}