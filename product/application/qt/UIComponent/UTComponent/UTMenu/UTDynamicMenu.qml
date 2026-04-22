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

    onModelChanged: _rebuild()

    function _rebuild() {
        // Remove all existing items (backwards to keep indices stable)
        for (var i = root.count - 1; i >= 0; --i)
            root.removeItem(root.itemAt(i))

        // Create new items from model
        for (var j = 0; j < model.length; ++j) {
            var data = model[j]
            if (data.separator) {
                root.addItem(_sepComponent.createObject(null))
            } else if (data.children && data.children.length > 0) {
                var submenu = Qt.createComponent("UTDynamicMenu.qml")
                    .createObject(root, { title: data.text, model: data.children })
                submenu.itemTriggered.connect(root.itemTriggered)
                root.addMenu(submenu)
            } else {
                var item = _itemComponent.createObject(null, { text: data.text })
                var action = data.action
                item.triggered.connect(function(a) {
                    return function() { root.itemTriggered(a) }
                }(action))
                root.addItem(item)
            }
        }
    }

    Component { id: _sepComponent;     UTMenuSeparator {} }
    Component { id: _itemComponent;    UTMenuItem {} }
}
