import QtQuick
import QtQuick.Controls
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTDialog - Themed modal dialog window.
 *
 * Provides consistent background, border overlay, and sensible defaults
 * for dialogs loaded via ViewFactory::loadQmlWindow().
 *
 * Usage:
 *   UTDialog {
 *       title: "My Dialog"
 *       width: 480; height: 360
 *       ColumnLayout { anchors.fill: parent; anchors.margins: 24; ... }
 *   }
 */
Window {
    id: dialog

    property var backgroundColorEnum: UIColorToken.Main_Window_Background
    property var borderColorEnum: UIColorToken.Window_Border

    visible: true
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint

    color: UTComponentUtil.getPlainUIColor(dialog.backgroundColorEnum, UIColorState.Normal)

    Rectangle {
        parent: Overlay.overlay ? Overlay.overlay : dialog.contentItem
        anchors.fill: parent
        z: 9999
        color: "transparent"
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(dialog.borderColorEnum, UIColorState.Normal)
    }
}
