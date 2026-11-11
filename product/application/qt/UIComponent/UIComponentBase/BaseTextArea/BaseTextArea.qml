import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

TextArea {
    id: control

    wrapMode: TextArea.Wrap
    selectByMouse: true

    // Keep Tab available for editing; Ctrl+Tab leaves the editor.
    Keys.onPressed: function(event) {
        if (!(event.modifiers & Qt.ControlModifier)
                || (event.modifiers & (Qt.AltModifier | Qt.MetaModifier))
                || (event.key !== Qt.Key_Tab && event.key !== Qt.Key_Backtab))
            return

        const forward = event.key !== Qt.Key_Backtab && !(event.modifiers & Qt.ShiftModifier)
        const target = control.nextItemInFocusChain(forward)
        if (target) {
            target.forceActiveFocus(forward ? Qt.TabFocusReason : Qt.BacktabFocusReason)
            event.accepted = true
        }
    }
}
