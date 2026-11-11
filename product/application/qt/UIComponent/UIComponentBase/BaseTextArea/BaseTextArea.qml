import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

TextArea {
    id: control

    property int borderRadius: 4
    property color backgroundColor: palette.base
    property color borderColor: palette.mid
    property real borderWidth: 1

    wrapMode: TextArea.Wrap
    selectByMouse: true

    background: Rectangle {
        radius: control.borderRadius
        color: control.backgroundColor
        border.width: control.borderWidth
        border.color: control.borderColor
    }

    function moveFocus(forward: bool): void {
        const target = control.nextItemInFocusChain(forward)
        if (target && target !== control)
            target.forceActiveFocus(forward ? Qt.TabFocusReason : Qt.BacktabFocusReason)
    }

    // Cocoa routes Control+Tab through shortcuts instead of ordinary key presses.
    // Qt's Meta modifier represents the physical Control key on macOS.
    Shortcut {
        sequence: Qt.platform.os === "osx" ? "Meta+Tab" : "Ctrl+Tab"
        context: Qt.WindowShortcut
        enabled: control.activeFocus
        onActivated: control.moveFocus(true)
    }

    Shortcut {
        sequence: Qt.platform.os === "osx" ? "Meta+Shift+Tab" : "Ctrl+Shift+Tab"
        context: Qt.WindowShortcut
        enabled: control.activeFocus
        onActivated: control.moveFocus(false)
    }
}
