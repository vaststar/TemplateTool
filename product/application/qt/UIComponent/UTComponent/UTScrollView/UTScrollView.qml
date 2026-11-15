import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTScrollView - Themed scroll view with sensible defaults.
 *
 * Defaults:
 * - clip: true
 * - contentWidth: availableWidth (no horizontal overflow)
 * - Horizontal scrollbar hidden by default
 *
 * Usage:
 *   UTScrollView {
 *       anchors.fill: parent
 *       ColumnLayout { width: parent.width; ... }
 *   }
 */
BaseScrollView {
    id: control

    ScrollBar.vertical: UTScrollBar {
        parent: control
        x: control.mirrored ? 0 : control.width - width
        y: control.topPadding
        height: control.availableHeight
        active: control.ScrollBar.horizontal.active
    }

    ScrollBar.horizontal: UTScrollBar {
        policy: ScrollBar.AlwaysOff
        parent: control
        x: control.leftPadding
        y: control.height - height
        width: control.availableWidth
        active: control.ScrollBar.vertical.active
    }
}
