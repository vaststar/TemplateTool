import QtQuick
import QtQuick.Controls.Basic

/**
 * Owns the complete model-driven contents of targetMenu.
 * Factories must be ready Components. A submenu factory must expose
 * title, model and itemTriggered(string action), matching a dynamic menu.
 * Presentation and theme choices belong to the supplied factories.
 */
QtObject {
    id: behavior

    required property Menu targetMenu
    property var model: []
    required property Component itemFactory
    required property Component separatorFactory
    required property Component submenuFactory

    signal itemTriggered(string action)

    property bool _complete: false

    onModelChanged: if (_complete) rebuild()
    Component.onCompleted: {
        _complete = true
        rebuild()
    }

    function clear() {
        // addMenu() creates a MenuItem separate from its submenu.
        // removeMenu() destroys both; removeItem() destroys only the item.
        for (let i = targetMenu.count - 1; i >= 0; --i) {
            const submenu = targetMenu.menuAt(i)
            if (submenu)
                targetMenu.removeMenu(submenu)
            else
                targetMenu.removeItem(targetMenu.itemAt(i))
        }
    }

    function create(factory, parent, properties) {
        if (!factory || factory.status !== Component.Ready) {
            console.warn("BaseDynamicMenuBehavior: menu factory is not ready")
            return null
        }
        const object = factory.createObject(parent, properties || {})
        if (!object)
            console.warn("BaseDynamicMenuBehavior: menu creation failed", factory.errorString())
        return object
    }

    function connectAction(item, action) {
        item.triggered.connect(function() { behavior.itemTriggered(action) })
    }

    function rebuild() {
        clear()
        const entries = model || []
        for (let i = 0; i < entries.length; ++i) {
            const entry = entries[i]
            if (entry.separator) {
                const separator = create(separatorFactory, targetMenu.contentItem)
                if (separator)
                    targetMenu.addItem(separator)
            } else if (entry.children && entry.children.length > 0) {
                const submenu = create(submenuFactory, targetMenu, {
                    title: entry.text, model: entry.children
                })
                if (submenu) {
                    submenu.itemTriggered.connect(behavior.itemTriggered)
                    targetMenu.addMenu(submenu)
                }
            } else {
                const item = create(itemFactory, targetMenu.contentItem, { text: entry.text })
                if (item) {
                    connectAction(item, entry.action)
                    targetMenu.addItem(item)
                }
            }
        }
    }
}
