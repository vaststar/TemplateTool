import QtQuick
import QtQuick.Controls
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTWindow - Themed top-level application window.
 *
 * Provides consistent background color and window-border overlay
 * matching the project design tokens.
 *
 * Usage:
 *   UTWindow {
 *       title: "My Window"
 *       width: 1024; height: 768
 *       header: MyTitleBar { }
 *       Loader { anchors.fill: parent; ... }
 *   }
 */
ApplicationWindow {
    id: root

    property var backgroundColorEnum: UIColorToken.Main_Window_Background
    property var borderColorEnum: UIColorToken.Window_Border

    visible: true
    color: UTComponentUtil.getPlainUIColor(root.backgroundColorEnum, UIColorState.Normal)

    Rectangle {
        parent: Overlay.overlay
        anchors.fill: parent
        z: 9999
        color: "transparent"
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(root.borderColorEnum, UIColorState.Normal)
    }
}
