import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseLabel {
    id: control

    property var fontEnum: UIFontToken.Body_Text
    property var colorEnum: UIColorToken.Content_Text
    property var colorState: UIColorState.Normal

    // ── Optional: link color token (enables link mode when set) ──
    property var linkColorEnum

    font: UTComponentUtil.getUIFont(fontEnum)
    color: UTComponentUtil.getPlainUIColor(colorEnum, colorState)

    // ── Link support: auto-enabled when linkColorEnum is set ──
    linkEnabled: linkColorEnum !== undefined
    linkColor: linkColorEnum !== undefined
               ? UTComponentUtil.getPlainUIColor(linkColorEnum,
                     _linkHovered ? UIColorState.Hovered : UIColorState.Normal)
               : color

    // ── Focus ring (auto-hidden when no activeFocus, zero render cost) ──
    UTFocusItem {
        target: control
    }
}
