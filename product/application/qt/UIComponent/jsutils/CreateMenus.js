.pragma library
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