import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

TabBar {
    id: control

    Binding {
        target: control.contentItem instanceof ListView ? control.contentItem : null
        property: "keyNavigationEnabled"
        value: false
        when: control.contentItem instanceof ListView
    }

    Keys.onPressed: function(event) {
        if ((event.key !== Qt.Key_Left && event.key !== Qt.Key_Right)
            || (event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier)))
            return

        const step = (event.key === Qt.Key_Right) !== control.mirrored ? 1 : -1
        for (let index = control.currentIndex + step; index >= 0 && index < control.count; index += step) {
            const tab = control.itemAt(index)
            if (tab && tab.enabled && tab.visible) {
                control.setCurrentIndex(index)
                tab.forceActiveFocus(step > 0 ? Qt.TabFocusReason : Qt.BacktabFocusReason)
                break
            }
        }
        event.accepted = true
    }
}
