.pragma library


function createMenu(parent, menuItem) {
    if (!menuItem) {
        return
    }
    var menu = Qt.createQmlObject(`
        import QtQuick; 
        import QtQuick.Controls; 
        Menu {}
        `, parent)
    menu.title = menuItem.name
    if (menuItem.subItems.length > 0) {
        for (let index = 0; index < menuItem.subItems.length; index++) {
            if (menuItem.subItems[index].subItems.length > 0) {
                createMenu(menu, menuItem.subItems[index])
            }
            else {
                var tempItem = Qt.createQmlObject(`
                    import QtQuick; 
                    import QtQuick.Controls; 
                    MenuItem {}
                    `, menu)
                tempItem.triggered.connect(menuItem.subItems[index].triggered)
                tempItem.text = menuItem.subItems[index].name
                menu.addItem(tempItem)
            }
        }
    }
    parent.addMenu(menu)
}