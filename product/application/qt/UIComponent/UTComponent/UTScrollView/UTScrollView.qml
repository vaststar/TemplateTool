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
}
