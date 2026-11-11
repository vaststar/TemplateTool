import QtQuick
import QtQuick.Controls

Window {
    id: dialog

    property color borderColor: palette.mid
    property real borderWidth: 1
    // Opt in when the dialog does not provide its own cancellation shortcut.
    property bool closeOnEscape: false

    visible: true
    modality: Qt.ApplicationModal
    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint | Qt.Tool

    Shortcut {
        sequence: "Escape"
        enabled: dialog.visible && dialog.closeOnEscape
        onActivated: dialog.close()
    }

    onVisibleChanged: {
        if (visible) {
            // visibleChanged precedes native window creation and display.
            Qt.callLater(dialog._activateWhenVisible)
        }
    }

    function _activateWhenVisible() {
        if (visible)
            requestActivate()
    }

    Rectangle {
        parent: Overlay.overlay ? Overlay.overlay : dialog.contentItem
        anchors.fill: parent
        z: 9999
        color: "transparent"
        border.width: dialog.borderWidth
        border.color: dialog.borderColor
    }
}
