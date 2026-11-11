import QtQuick
import QtQuick.Controls

// Generic tree row. Subclasses add theming; callers wire `onClicked`.
// Drag is enabled iff `dragMimeData` is non-null; drop is handled by BaseTreeView.
TreeViewDelegate {
    id: control

    property font textFont: font
    property font indicatorFont: font
    property color textColor: palette.windowText
    property color indicatorColor: palette.windowText
    property color backgroundColor: "transparent"
    property real backgroundRadius: 2
    property color borderColor: palette.highlight
    property real borderWidth: isDropTarget ? 2 : 0

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

    implicitWidth: treeView ? treeView.width : 1
    implicitHeight: Math.max(
        contentItem ? contentItem.implicitHeight : 0,
        indicator   ? indicator.implicitHeight   : 0
    ) + topPadding + bottomPadding
    indentation: 8
    leftMargin: 8
    spacing: 6
    topPadding: 4
    bottomPadding: 4

    indicator: Item {
        x: control.leftMargin + control.depth * control.indentation
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: 20
        implicitHeight: 20
        visible: control.hasChildren

        Label {
            anchors.centerIn: parent
            enabled: false
            text: "\u25B6"
            font: control.indicatorFont
            color: control.indicatorColor
            textFormat: Text.PlainText
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            rotation: control.expanded ? 90 : 0
        }
    }

    contentItem: Label {
        text: control.text
        font: control.textFont
        color: control.textColor
        textFormat: Text.PlainText
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: control.backgroundColor
        border.width: control.borderWidth
        border.color: control.borderColor
        radius: control.backgroundRadius
    }

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
