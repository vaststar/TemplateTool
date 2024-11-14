.pragma library

function createMenu(parent, menuItem) {
    if (!menuItem) {
        return
    }
    var menu = Qt.createQmlObject('import QtQuick; import QtQuick.Controls; Menu {}', parent)
    menu.title = menuItem.name
    if (menuItem.subItems.length > 0) {
        for (let index = 0; index < menuItem.subItems.length; index++) {
            if (menuItem.subItems[index].subItems.length > 0) {
                createMenu(menu, menuItem.subItems[index])
            }
            else {
                var tempItem = Qt.createQmlObject('import QtQuick; import QtQuick.Controls; MenuItem {}', menu)
                tempItem.text = menuItem.subItems[index].name
                menu.addItem(tempItem)
            }
        }
    }
    parent.addMenu(menu)
}

function createMenus(parent, menuItems) {
    for (let index = 0; index < menuItems.length; index++) {
        var menu = Qt.createQmlObject('import QtQuick 2.13; import QtQuick.Controls 2.13; Menu {}',
            parent)
            menu.title = menuItems[index].name
        createMenuItems(menu, menuItems[index])
        parent.insertMenu(index, menu)
    }
}

function createMenuItems(parent, menu) {
    if (menu.subItems.length > 0) {
        for (let index = 0; index < menu.subItems.length; index++) {
            if (menu.subItems[index].subItems.length > 0){
                var submenu = Qt.createQmlObject('import QtQuick 2.13; import QtQuick.Controls 2.13; Menu {}',
                    parent)
                submenu.title = menu.subItems[index].name
                createMenuItems(submenu, menu.subItems[index])
                parent.addMenu(submenu)
            } else {
                var menuItem = Qt.createQmlObject('import QtQuick 2.13; import QtQuick.Controls 2.13; MenuItem {}',
                    parent)
                menuItem.text = menu.subItems[index].name
                parent.addItem(menuItem)
            }
        }
    }
}