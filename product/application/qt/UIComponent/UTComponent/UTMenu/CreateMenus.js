.pragma library

function createMenu(parent, menuItem) {
    if (!menuItem) {
        return
    }
    var menu_component = Qt.createComponent("UTMenu.qml")
    var menu = menu_component.createObject(parent, { title: menuItem.name })
    if (menuItem.subItems.length > 0) {
        for (let index = 0; index < menuItem.subItems.length; index++) {
            if (menuItem.subItems[index].subItems.length > 0) {
                createMenu(menu, menuItem.subItems[index])
            }
            else {
                var subitem_component = Qt.createComponent("UTMenuItem.qml")
                var subitem = subitem_component.createObject(null, { text: menuItem.subItems[index].name})
                subitem.triggered.connect(menuItem.subItems[index].triggered)
                menu.addItem(subitem)
            }
        }
    }
    parent.addMenu(menu)
}