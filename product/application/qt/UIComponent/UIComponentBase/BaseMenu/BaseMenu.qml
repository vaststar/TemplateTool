import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Templates as T

Menu {
    id: control

    property color backgroundColor: palette.window
    property color borderColor: palette.mid
    property real borderWidth: 1
    property int borderRadius: 8
    property int menuPadding: 4

    padding: menuPadding
    overlap: 4

    // Open at a caller-provided position and initialize keyboard navigation.
    function popupAt(x, y, fromKeyboard) {
        popup(x, y)
        if (!fromKeyboard)
            return
        for (let i = 0; i < count; ++i) {
            const item = itemAt(i)
            if (item instanceof T.MenuItem && item.visible && item.enabled) {
                currentIndex = i
                return
            }
        }
    }

    background: Rectangle {
        implicitWidth: 200
        radius: control.borderRadius
        color: control.backgroundColor
        border.width: control.borderWidth
        border.color: control.borderColor
        layer.enabled: true
    }
}
