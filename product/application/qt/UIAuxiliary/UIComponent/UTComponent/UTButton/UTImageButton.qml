import QtQuick

Item {
    id: root
    width: image.width
    height: image.height

    property url source: ""                // 默认图片
    property url sourcePressed: ""         // 按下时的图片
    property url sourceHovered: ""         // hover 时图片
    signal clicked()

    property bool pressed: false
    property bool hovered: false

    Image {
        id: image
        anchors.fill: parent
        source: root.pressed && root.sourcePressed ? root.sourcePressed :
                root.hovered && root.sourceHovered ? root.sourceHovered :
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