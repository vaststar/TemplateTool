import QtQuick
import UIComponentBase 1.0
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

    BaseDynamicMenuBehavior {
        targetMenu: root
        model: root.model
        itemFactory: Component { UTMenuItem {} }
        separatorFactory: Component { UTMenuSeparator {} }
        // Dynamic resolution avoids an eager self-reference in this QML type.
        submenuFactory: Qt.createComponent(Qt.resolvedUrl("UTDynamicMenu.qml"))
        onItemTriggered: function(action) { root.itemTriggered(action) }
    }
}
