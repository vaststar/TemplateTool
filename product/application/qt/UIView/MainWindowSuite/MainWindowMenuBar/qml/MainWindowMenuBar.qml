import QtQuick
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0

MenuBar {
    id: root
    property MainWindowMenuBarController controller: MainWindowMenuBarController{}

    Connections {
        target: controller
        function onControllerInitialized() { createMenuItems() }
    }

    function createMenuItems() {
        for (let i = 0; i < controller.menuModel.length; i++) {
            var entry = controller.menuModel[i]
            var menu = _menuComponent.createObject(root, {
                title: entry.text,
                model: entry.children || []
            })
            menu.itemTriggered.connect(function(action) {
                controller.handleMenuAction(action)
            })
            root.addMenu(menu)
        }
    }

    Component {
        id: _menuComponent
        UTDynamicMenu {}
    }
}
