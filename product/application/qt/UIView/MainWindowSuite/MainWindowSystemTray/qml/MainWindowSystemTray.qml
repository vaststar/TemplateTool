import QtQuick
import QtQuick.Controls
import Qt.labs.platform
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

SystemTrayIcon {
    id: root
    property MainWindowSystemTrayController controller: MainWindowSystemTrayController{}

    visible: true
    icon.source: UTComponentUtil.getImageResourcePath(UIAssetImageToken.Logo)

    onActivated: function(reason) {
        if (reason === SystemTrayIcon.Trigger || reason === SystemTrayIcon.DoubleClick) {
            root.controller.activateMainWindow()
        }
    }

     menu: Menu {
        MenuItem {
            text: qsTr("Quit")
            onTriggered: root.controller.quit()
        }
    }
}
