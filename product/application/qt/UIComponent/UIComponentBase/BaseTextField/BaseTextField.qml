import QtQuick
import QtQuick.Controls.Basic

/**
 * BaseTextField - Themeable single-line text input.
 *
 * Provides keyboard focus management and a clean API surface.
 * Subclasses supply themed colors via property overrides.
 */
TextField {
    id: control

    property int borderRadius: 4
    property real fieldHeight: 32

    implicitHeight: fieldHeight

    verticalAlignment: TextInput.AlignVCenter
    leftPadding: 12
    rightPadding: 12

    selectByMouse: true
    selectedTextColor: palette.highlightedText
    selectionColor: palette.highlight
}
