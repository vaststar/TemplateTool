import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent
import UIResourceLoader 1.0

/**
 * RecordingFloatingBar - Small always-on-top control bar for fullscreen recording.
 *
 * Shows: recording indicator, duration, Pause/Resume, Stop, Abort buttons.
 * Used when recording in fullscreen mode (no region border needed).
 * Positioned at top-center of the screen.
 */
Window {
    id: barWindow
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus
    width: barContent.width + 24
    height: 44
    x: Screen.width / 2 - width / 2
    y: 12
    visible: false
    color: "transparent"

    onVisibleChanged: {
        if (visible) {
            raise()
        }
    }

    required property var controller

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

    // Lifecycle managed by RecordingContent — no self-destroy here.
    // RecordingContent listens for onRecordingStateChanged and
    // calls close()/destroy() on the activeFloatingBar property.

    Rectangle {
        anchors.fill: parent
        radius: 8
        color: "#2D2D2D"
        border.color: "#555555"
        border.width: 1

        // Drag area (declared before RowLayout → lower z-order, buttons get clicks first)
        MouseArea {
            id: dragArea
            anchors.fill: parent
            property point clickPos

            onPressed: function(mouse) {
                clickPos = Qt.point(mouse.x, mouse.y)
            }
            onPositionChanged: function(mouse) {
                barWindow.x += mouse.x - clickPos.x
                barWindow.y += mouse.y - clickPos.y
            }
        }

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
                    running: controller.isRecording && !controller.isPaused
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
                font.family: UTComponentUtil.getUIFont(UIFontToken.Monospace_Text).family
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
                onClicked: controller.stopRecording()
            }

            // Abort
            UTToolButton {
                text: "✕"
                toolTipText: qsTr("Abort recording")
                onClicked: controller.abortRecording()
            }
        }
    }
}
