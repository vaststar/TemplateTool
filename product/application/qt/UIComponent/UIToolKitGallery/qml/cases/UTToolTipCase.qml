import QtQuick
import UTComponent 1.0
import "../common"

GalleryCase {
    id: root
    objectName: "utToolTipCase"

    title: "UTToolTip"

    onVisibleChanged: {
        if (!visible)
            toolTip.close()
    }
    Component.onDestruction: toolTip.close()

    GalleryDemo {
        title: qsTr("Hover / Keyboard focus")

        UTButton {
            id: detailsButton
            objectName: "toolTipButton"
            text: qsTr("Details")
            hoverEnabled: true
            focusPolicy: Qt.StrongFocus

            HoverHandler {
                id: buttonHover
                objectName: "toolTipHoverHandler"
            }

            UTToolTip {
                id: toolTip
                objectName: "demoToolTip"
                parent: detailsButton
                text: qsTr("Contextual details")
                visible: root.visible && (pointerActive || detailsButton.activeFocus)

                readonly property bool pointerActive: detailsButton.hovered && buttonHover.hovered
                // UTToolTip adds (6, 14); focus places it 8 px below the button.
                readonly property point triggerPosition: detailsButton.mapToItem(
                    toolTip.parent,
                    pointerActive ? buttonHover.point.position.x : -6,
                    pointerActive ? buttonHover.point.position.y : detailsButton.height - 6)
                cursorX: triggerPosition.x
                cursorY: triggerPosition.y
            }
        }
    }
}
