import QtQuick
import QtQuick.Controls
Menu {
    id: rootMenu
    property var menuItem

    title: menuItem.name
    Component.onCompleted: {
        // createSubMenus(rootMenu, menuItem)
    }

    // function createSubMenus(parent, data) {
    //     for (var i = 0; i < data.subItems.length; i++) {
    //         if (data.subItems.length > 0) {

    //         }
    //     }
    // }

    Repeater {
        model: menuItem.subItems
        Loader {
            sourceComponent: modelData.subItems.length > 0 ? subMenuComponent : menuItemComponent
        }
    }

    // 子菜单组件
    Component {
        id: subMenuComponent
        // 递归引用自身
        UTMenu {
            menuItem: modelData
        }
    }

    // 菜单项组件
    Component {
        id: menuItemComponent
        MenuItem {
            text: modelData.name
            onTriggered: console.log("Action: " + modelData.action)
        }
    }
}