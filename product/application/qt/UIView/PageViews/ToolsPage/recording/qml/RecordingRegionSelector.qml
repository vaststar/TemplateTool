import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent

/**
 * RecordingRegionSelector - Full-screen overlay for selecting a screen region,
 * then transitions to a click-through recording border with a floating control bar.
 *
 * Two phases in one Window:
 *   Phase 1 (selecting) : Full-screen, semi-transparent, mouse interactive → drag to select region
 *   Phase 2 (recording) : Shrinks to selection rect, click-through border + floating control bar
 */
Window {
    id: selectorWindow
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    x: Screen.virtualX
    y: Screen.virtualY
    width: Screen.width
    height: Screen.height
    visible: false
    color: "transparent"

    required property var controller

    signal recordingFinished(string filePath)

    // === State ===
    property bool isSelecting: false
    property bool selectionDone: false
    property bool isRecording: false

    // Selection coordinates (screen-absolute)
    property real selStartX: 0
    property real selStartY: 0
    property real selEndX: 0
    property real selEndY: 0

    // Computed selection rect
    property real selX: Math.min(selStartX, selEndX)
    property real selY: Math.min(selStartY, selEndY)
    property real selW: Math.abs(selEndX - selStartX)
    property real selH: Math.abs(selEndY - selStartY)

    // Close on Escape = abort
    Shortcut {
        sequence: "Escape"
        onActivated: abortRecording()
    }

    // Listen to controller state
    Connections {
        target: controller

        function onRecordingCompleted(filePath) {
            selectorWindow.recordingFinished(filePath)
            cleanup()
        }

        function onErrorOccurred(message) {
            console.error("Recording error:", message)
            cleanup()
        }

        function onRecordingStateChanged() {
            if (!controller.isRecording && selectorWindow.isRecording) {
                cleanup()
            }
        }
    }

    function formatDuration(seconds) {
        var h = Math.floor(seconds / 3600)
        var m = Math.floor((seconds % 3600) / 60)
        var s = seconds % 60
        if (h > 0) {
            return String(h).padStart(2, '0') + ":" +
                   String(m).padStart(2, '0') + ":" +
                   String(s).padStart(2, '0')
        }
        return String(m).padStart(2, '0') + ":" + String(s).padStart(2, '0')
    }

    // Screen-absolute coordinates for the selected region
    property real absSelX: Screen.virtualX + selX
    property real absSelY: Screen.virtualY + selY

    function startRegionRecording() {
        isRecording = true

        // Keep the window fullscreen but make it click-through.
        // On Wayland, shrinking/moving a window is unreliable — the compositor
        // may reposition it, causing the "jump to fullscreen" or "shifted position"
        // bugs.  Instead we stay fullscreen + transparent-for-input and draw
        // the recording border at the selection coordinates inside this window.
        selectorWindow.flags = Qt.FramelessWindowHint
                             | Qt.WindowStaysOnTopHint
                             | Qt.Tool
                             | Qt.WindowTransparentForInput
        selectorWindow.color = "transparent"

        // Show floating control bar (screen-absolute coordinates)
        floatingBar.x = absSelX
        floatingBar.y = absSelY + selH + 8
        // If bar would go off-screen, put it above
        if (floatingBar.y + floatingBar.height > Screen.virtualY + Screen.height) {
            floatingBar.y = absSelY - floatingBar.height - 8
        }
        floatingBar.show()

        // Start the actual recording with screen-absolute coordinates
        controller.startRegionRecording(absSelX, absSelY, selW, selH)
    }

    function stopRecording() {
        controller.stopRecording()
        // cleanup will be called via onRecordingCompleted or onRecordingStateChanged
    }

    function abortRecording() {
        if (isRecording) {
            controller.abortRecording()
        }
        cleanup()
    }

    function cleanup() {
        isRecording = false
        floatingBar.close()
        selectorWindow.close()
        selectorWindow.destroy()
    }

    // ==========================================
    // Phase 1: Full-screen selection overlay
    // ==========================================

    // Dark overlay canvas
    Canvas {
        id: overlayCanvas
        anchors.fill: parent
        visible: !isRecording

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            // Semi-transparent dark overlay
            ctx.fillStyle = "rgba(0, 0, 0, 0.4)"
            ctx.fillRect(0, 0, width, height)

            // Cut out the selection area
            if (isSelecting || selectionDone) {
                ctx.globalCompositeOperation = "destination-out"
                ctx.fillStyle = "rgba(0, 0, 0, 1)"
                ctx.fillRect(selX, selY, selW, selH)
                ctx.globalCompositeOperation = "source-over"

                // Draw selection border
                ctx.strokeStyle = "#00AAFF"
                ctx.lineWidth = 2
                ctx.setLineDash([5, 5])
                ctx.strokeRect(selX, selY, selW, selH)
                ctx.setLineDash([])

                // Size indicator
                if (selW > 60 && selH > 20) {
                    var sizeText = Math.round(selW) + " × " + Math.round(selH)
                    ctx.font = "12px Arial"
                    ctx.fillStyle = "#00AAFF"
                    ctx.fillText(sizeText, selX + 5, selY + 15)
                }
            }
        }
    }

    // Mouse area for selection (only during phase 1)
    MouseArea {
        id: selectionMouseArea
        anchors.fill: parent
        visible: !isRecording
        cursorShape: selectionDone ? Qt.ArrowCursor : Qt.CrossCursor

        onPressed: function(mouse) {
            if (selectionDone) return
            isSelecting = true
            selStartX = mouse.x
            selStartY = mouse.y
            selEndX = mouse.x
            selEndY = mouse.y
            overlayCanvas.requestPaint()
        }

        onPositionChanged: function(mouse) {
            if (!isSelecting) return
            selEndX = mouse.x
            selEndY = mouse.y
            overlayCanvas.requestPaint()
        }

        onReleased: function(mouse) {
            if (!isSelecting) return
            isSelecting = false
            selEndX = mouse.x
            selEndY = mouse.y

            // Minimum selection size
            if (selW > 20 && selH > 20) {
                selectionDone = true
            }
            overlayCanvas.requestPaint()
        }
    }

    // Confirm / Cancel toolbar (appears after selection, before recording)
    Rectangle {
        id: confirmBar
        visible: selectionDone && !isRecording
        x: Math.max(10, Math.min(selX, parent.width - width - 10))
        y: selY + selH + 10 < parent.height - 60 ? selY + selH + 10 : selY - height - 10
        width: confirmRow.width + 20
        height: 44
        radius: 8
        color: "#2D2D2D"
        border.color: "#555555"
        border.width: 1

        RowLayout {
            id: confirmRow
            anchors.centerIn: parent
            spacing: 8

            Text {
                text: Math.round(selW) + " × " + Math.round(selH)
                color: "#AAAAAA"
                font.pixelSize: 12
                font.family: "monospace"
            }

            Rectangle { width: 1; height: 24; color: "#555555" }

            ToolButton {
                text: "🎬 Record"
                onClicked: startRegionRecording()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Start recording this region")
                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#44AA44" : (parent.hovered ? "#448844" : "transparent")
                }
            }

            ToolButton {
                text: "↩ Reselect"
                onClicked: {
                    selectionDone = false
                    selStartX = 0; selStartY = 0
                    selEndX = 0; selEndY = 0
                    overlayCanvas.requestPaint()
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Draw a new selection")
                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#555555" : (parent.hovered ? "#444444" : "transparent")
                }
            }

            Rectangle { width: 1; height: 24; color: "#555555" }

            ToolButton {
                text: "✕"
                onClicked: cleanup()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Cancel")
                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#AA4444" : (parent.hovered ? "#884444" : "transparent")
                }
            }
        }
    }

    // Instructions overlay
    Rectangle {
        anchors.centerIn: parent
        width: instructionText.width + 40
        height: instructionText.height + 20
        radius: 8
        color: "#CC000000"
        visible: !selectionDone && !isSelecting && !isRecording

        Text {
            id: instructionText
            anchors.centerIn: parent
            text: qsTr("Click and drag to select recording region\nPress ESC to cancel")
            color: "white"
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
        }
    }

    // ==========================================
    // Phase 2: Recording border (click-through)
    // ==========================================

    // Red border for recording area (visible when recording)
    // Window stays fullscreen, so we position the border at the selection rect.
    // Border is drawn OUTSIDE the selection so it doesn't contaminate the recorded crop.
    Rectangle {
        id: recordingBorder
        readonly property int bw: 3
        x: selX - bw
        y: selY - bw
        width: selW + bw * 2
        height: selH + bw * 2
        visible: isRecording
        color: "transparent"
        border.color: "red"
        border.width: bw

        SequentialAnimation on border.color {
            running: isRecording && !controller.isPaused
            loops: Animation.Infinite
            ColorAnimation { to: "#FF0000"; duration: 800 }
            ColorAnimation { to: "#FF6666"; duration: 800 }
        }
    }

    // Corner markers for recording area (outside the selection)
    Repeater {
        model: isRecording ? 4 : 0
        Rectangle {
            width: 12
            height: 12
            color: "red"
            radius: 2
            x: selX + ((index % 2 === 0) ? -width : selW)
            y: selY + ((index < 2) ? -height : selH)
        }
    }

    // ==========================================
    // Floating Control Bar (separate window)
    // ==========================================
    Window {
        id: floatingBar
        flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
        width: barContent.width + 24
        height: 44
        visible: false
        color: "transparent"

        Rectangle {
            anchors.fill: parent
            radius: 8
            color: "#2D2D2D"
            border.color: "#555555"
            border.width: 1

            RowLayout {
                id: barContent
                anchors.centerIn: parent
                spacing: 8

                // Recording indicator dot
                Rectangle {
                    width: 12
                    height: 12
                    radius: 6
                    color: controller.isPaused ? "gray" : "red"

                    SequentialAnimation on opacity {
                        running: selectorWindow.isRecording && !controller.isPaused
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 500 }
                        NumberAnimation { to: 1.0; duration: 500 }
                    }
                }

                // Duration
                Text {
                    text: formatDuration(controller.recordingDuration)
                    color: "#FFFFFF"
                    font.pixelSize: 13
                    font.family: "Consolas"
                }

                Rectangle { width: 1; height: 24; color: "#555555" }

                // Pause / Resume
                UTToolButton {
                    text: controller.isPaused ? "▶" : "⏸"
                    toolTipText: controller.isPaused ? qsTr("Resume") : qsTr("Pause")
                    onClicked: {
                        if (controller.isPaused)
                            controller.resumeRecording()
                        else
                            controller.pauseRecording()
                    }
                }

                // Stop
                UTToolButton {
                    text: "⏹"
                    toolTipText: qsTr("Stop recording")
                    onClicked: stopRecording()
                }

                // Abort
                UTToolButton {
                    text: "✕"
                    toolTipText: qsTr("Abort recording")
                    onClicked: abortRecording()
                }
            }
        }
    }
}
