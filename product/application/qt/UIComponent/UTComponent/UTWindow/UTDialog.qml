import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTDialog - Themed modal dialog window.
 *
 * Provides consistent background, border overlay, and sensible defaults
 * for dialogs loaded via ViewFactory::createQmlWindow().
 *
 * Usage:
 *   UTDialog {
 *       title: "My Dialog"
 *       width: 480; height: 360
 *       ColumnLayout { anchors.fill: parent; anchors.margins: 24; ... }
 *   }
 */
BaseDialog {
    id: dialog

    property var backgroundColorEnum: UIColorToken.Main_Window_Background
    property var borderColorEnum: UIColorToken.Window_Border
    color: UTComponentUtil.getPlainUIColor(dialog.backgroundColorEnum, UIColorState.Normal)
    borderColor: UTComponentUtil.getPlainUIColor(dialog.borderColorEnum, UIColorState.Normal)
}
