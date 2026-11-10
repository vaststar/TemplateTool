import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0

BaseImageButton {
    id: control

    radius: Math.min(control.width, control.height) / 2
    clip: false

    UTFocusItem {
        target: control
        focusRadius: control.radius
    }
}
