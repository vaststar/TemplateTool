import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseLabel {
    id: control

    // ── Required tokens ──
    required property var fontEnum
    required property var colorEnum
    property var colorState: UIColorState.Normal

    // ── Optional: link color token (enables link mode when set) ──
    property var linkColorEnum

    // Internal: track link hover state
    property bool _linkHovered: false
    
    font: UTComponentUtil.getUIFont(fontEnum)
    color: UTComponentUtil.getPlainUIColor(colorEnum, colorState)

    // ── Link support: auto-enabled when linkColorEnum is set ──
    textFormat: linkColorEnum !== undefined ? Text.StyledText : Text.PlainText
    linkColor: linkColorEnum !== undefined
               ? UTComponentUtil.getPlainUIColor(linkColorEnum,
                     _linkHovered ? UIColorState.Hovered : UIColorState.Normal)
               : color
    activeFocusOnTab: linkColorEnum !== undefined

    onHoveredLinkChanged: _linkHovered = (hoveredLink !== "")

    // ── Accessibility ──
    Accessible.role: linkColorEnum !== undefined ? Accessible.Link : Accessible.StaticText
    Accessible.name: linkColorEnum !== undefined ? text.replace(/<[^>]*>/g, "") : text

    // ── Keyboard: Enter/Return activates first link ──
    Keys.onPressed: (event) => {
        if (linkColorEnum !== undefined &&
            (event.key === Qt.Key_Return || event.key === Qt.Key_Enter)) {
            const match = text.match(/href=["']([^"']*)["']/)
            if (match) {
                control.linkActivated(match[1])
                event.accepted = true
            }
        }
    }

    // ── Focus ring (auto-hidden when no activeFocus, zero render cost) ──
    UTFocusItem {
        target: control
    }
}
