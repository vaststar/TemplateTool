import QtQuick
import UTComponent 1.0

/**
 * UTDynamicMenu - Data-driven multi-level context menu.
 *
 * Accepts a plain JS array as model. Each element is an object:
 *   { text: string, action: string }             — leaf item
 *   { text: string, children: [ ... ] }           — submenu
 *   { separator: true }                           — separator line
 *
 * Usage:
 *   UTDynamicMenu {
 *       model: [
 *           { text: "Open",   action: "open" },
 *           { text: "Export", children: [
 *               { text: "PNG", action: "export_png" },
 *               { text: "PDF", action: "export_pdf" }
 *           ]},
 *           { separator: true },
 *           { text: "Delete", action: "delete" }
 *       ]
 *       onItemTriggered: function(action) { controller.handleAction(action) }
 *   }
 */
UTMenu {
    id: root

    property var model: []
    signal itemTriggered(string action)

    Instantiator {
        model: root.model
        delegate: QtObject {
            property var itemData: root.model[index]

            Component.onCompleted: {
                if (itemData.separator) {
                    var sep = _sepComponent.createObject(root)
                    root.addItem(sep)
                } else if (itemData.children && itemData.children.length > 0) {
                    var comp = Qt.createComponent("UTDynamicMenu.qml")
                    var submenu = comp.createObject(root, {
                        title: itemData.text,
                        model: itemData.children
                    })
                    submenu.itemTriggered.connect(root.itemTriggered)
                    root.addMenu(submenu)
                } else {
                    var item = _itemComponent.createObject(root, {
                        text: itemData.text
                    })
                    var action = itemData.action
                    item.triggered.connect(function() {
                        root.itemTriggered(action)
                    })
                    root.addItem(item)
                }
            }
        }
    }

    Component { id: _sepComponent;     UTMenuSeparator {} }
    Component { id: _itemComponent;    UTMenuItem {} }
}
