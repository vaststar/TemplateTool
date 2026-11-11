import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

TextArea {
    id: control

    wrapMode: TextArea.Wrap
    selectByMouse: true

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
