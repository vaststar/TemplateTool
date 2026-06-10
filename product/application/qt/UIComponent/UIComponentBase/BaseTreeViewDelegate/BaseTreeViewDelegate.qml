import QtQuick
import QtQuick.Controls

// Generic tree row. Subclasses add theming; callers wire `onClicked`.
// Drag is enabled iff `dragMimeData` is non-null; drop is handled by BaseTreeView.
TreeViewDelegate {
    id: control

    property var dragMimeData: null
    property Item dragPreviewSource: _defaultPreview
    property real dragHotSpotX: 18
    property real dragHotSpotY: 12

    readonly property bool dragActive: _dragHandler.active

    // True when this row is the current drop target (set by BaseTreeView).
    // Subclasses bind their background/border to render the drop highlight.
    readonly property bool isDropTarget: treeView && treeView.dropTargetRow === row

    focusPolicy: Qt.NoFocus
    z: (current && treeView.activeFocus) ? 1 : 0

    Drag.active: dragMimeData !== null && _dragHandler.active
    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.MoveAction
    Drag.mimeData: dragMimeData || ({})
    Drag.hotSpot.x: dragHotSpotX
    Drag.hotSpot.y: dragHotSpotY

    Item {
        id: _defaultPreview
        visible: false
        layer.enabled: true
        opacity: 0.85
        width:  _previewText.implicitWidth + 16
        height: _previewText.implicitHeight + 8
        Rectangle {
            anchors.fill: parent
            radius: 4
            color: "#80000000"
        }
        Text {
            id: _previewText
            anchors.centerIn: parent
            text: control.text
            color: "white"
        }
    }

    TapHandler {
        enabled: control.dragMimeData !== null
        acceptedButtons: Qt.LeftButton
        onPressedChanged: {
            if (pressed) {
                const src = control.dragPreviewSource || control
                src.grabToImage(function(r) { control.Drag.imageSource = r.url })
            }
        }
    }

    DragHandler {
        id: _dragHandler
        enabled: control.dragMimeData !== null
        target: null
        dragThreshold: 6
        grabPermissions: PointerHandler.CanTakeOverFromAnything
    }
}
