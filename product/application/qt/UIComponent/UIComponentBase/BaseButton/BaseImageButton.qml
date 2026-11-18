import QtQuick
import UIComponentBase 1.0

BaseButton {
    id: control

    required property url source
    property url sourcePressed: ""
    property url sourceHovered: ""

    padding: 0
    horizontalPadding: 0
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    background: null

    contentItem: Image {
        fillMode: Image.PreserveAspectFit
        mipmap: true
        source: control.down && control.sourcePressed.toString() !== "" ? control.sourcePressed :
                control.hovered && control.sourceHovered.toString() !== "" ? control.sourceHovered :
                control.source
    }
}
