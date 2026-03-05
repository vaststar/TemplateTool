import QtQuick

Item {
    id: root
    width: image.width
    height: image.height

    required property url source                 // 默认图片（必填）
    property url sourcePressed: ""                // 按下时的图片，未设置则回退 source
    property url sourceHovered: ""                // hover 时图片，未设置则回退 source
    signal clicked()

    property bool pressed: false
    property bool hovered: false

    Image {
        id: image
        anchors.fill: parent
        source: root.pressed && root.sourcePressed.toString() !== "" ? root.sourcePressed :
                root.hovered && root.sourceHovered.toString() !== "" ? root.sourceHovered :
                root.source
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPressed: root.pressed = true
        onReleased: {
            root.pressed = false
            if (containsMouse)
                root.clicked()
        }
        onExited: root.hovered = false
        onEntered: root.hovered = true
        onCanceled: root.pressed = false
    }
}
// UTImageButton.qml