import QtQuick
import QtQml.Models
import Qt.labs.platform as Platform
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Platform.SystemTrayIcon {
    id: root
    property MainWindowSystemTrayController controller: MainWindowSystemTrayController{}

    visible: true
    icon.source: UTComponentUtil.getImageResourcePath(UIAssetImageToken.Logo)

    onActivated: function(reason) {
        // On macOS, clicking the tray icon opens its native menu.
        if (Qt.platform.os !== "osx"
                && (reason === Platform.SystemTrayIcon.Trigger || reason === Platform.SystemTrayIcon.DoubleClick)) {
            root.controller.activateMainWindow()
        }
    }

    menu: Platform.Menu {
        id: trayMenu

        Instantiator {
            model: root.controller.menuItems

            delegate: Platform.MenuItem {
                required property var modelData

                text: modelData.title
                enabled: modelData.enabled
                separator: modelData.separator
                onTriggered: root.controller.handleMenuAction(modelData.actionId)
            }

            onObjectAdded: function(index, object) {
                trayMenu.insertItem(index, object)
            }
            onObjectRemoved: function(index, object) {
                trayMenu.removeItem(object)
            }
        }
    }
}
