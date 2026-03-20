import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent
import UIResourceLoader 1.0

/**
 * Recording Content - Screen recording controls and GIF conversion
 */
Item {
    id: root

    required property var controller

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        // === FFmpeg Status ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: statusRow.implicitHeight + 24
            color: controller.isFFmpegAvailable()
                   ? UTComponentUtil.getPlainUIColor(UIColorToken.Status_Success_BG, UIColorState.Normal)
                   : UTComponentUtil.getPlainUIColor(UIColorToken.Status_Error_BG, UIColorState.Normal)
            radius: 8

            RowLayout {
                id: statusRow
                anchors {
                    fill: parent
                    margins: 12
                }
                spacing: 12

                UTText {
                    text: controller.isFFmpegAvailable() ? "✅" : "⚠️"
                    font.pixelSize: 20
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    UTText {
                        text: controller.isFFmpegAvailable()
                              ? qsTr("FFmpeg is available")
                              : qsTr("FFmpeg not found")
                        fontEnum: UIFontToken.Body_Text_Bold
                    }

                    UTText {
                        visible: !controller.isFFmpegAvailable()
                        text: qsTr("Install FFmpeg to enable screen recording features")
                        fontEnum: UIFontToken.Caption_Text
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    UTText {
                        visible: controller.isFFmpegAvailable()
                        text: controller.getFFmpegPath()
                        fontEnum: UIFontToken.Caption_Text
                        colorEnum: UIColorToken.Content_Secondary
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // === Recording Controls ===
        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Screen Recording")
            enabled: controller.isFFmpegAvailable()

            ColumnLayout {
                anchors.fill: parent
                spacing: 16

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    UTButton {
                        text: controller.isRecording
                              ? (controller.isPaused ? qsTr("Resume") : qsTr("Pause"))
                              : qsTr("Start Recording")

                        onClicked: {
                            if (controller.isRecording) {
                                if (controller.isPaused) {
                                    controller.resumeRecording()
                                } else {
                                    controller.pauseRecording()
                                }
                            } else {
                                controller.startRecording(modeCombo.currentValue)
                            }
                        }
                    }

                    UTButton {
                        text: qsTr("Stop")
                        enabled: controller.isRecording
                        onClicked: controller.stopRecording()
                    }

                    Item { Layout.fillWidth: true }

                    UTText {
                        text: qsTr("Mode:")
                        fontEnum: UIFontToken.Body_Text
                    }

                    ComboBox {
                        id: modeCombo
                        model: [
                            { text: qsTr("Full Screen"), value: "fullscreen" },
                            { text: qsTr("Select Region"), value: "region" }
                        ]
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: 0
                        enabled: !controller.isRecording
                        implicitWidth: 150
                    }
                }

                // Recording status
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    visible: controller.isRecording
                    color: UTComponentUtil.getPlainUIColor(UIColorToken.Status_Error_BG, UIColorState.Normal)
                    radius: 8

                    RowLayout {
                        anchors {
                            fill: parent
                            margins: 12
                        }
                        spacing: 12

                        Rectangle {
                            width: 16
                            height: 16
                            radius: 8
                            color: controller.isPaused ? "gray" : "red"

                            SequentialAnimation on opacity {
                                running: controller.isRecording && !controller.isPaused
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.3; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                        }

                        UTText {
                            text: controller.isPaused ? qsTr("Paused") : qsTr("Recording")
                            fontEnum: UIFontToken.Body_Text_Bold
                        }

                        Item { Layout.fillWidth: true }

                        UTText {
                            text: formatDuration(controller.recordingDuration)
                            fontEnum: UIFontToken.Body_Text_Bold
                            font.family: "monospace"
                        }
                    }
                }
            }
        }

        // === GIF Conversion ===
        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Convert to GIF")
            enabled: controller.isFFmpegAvailable()

            ColumnLayout {
                anchors.fill: parent
                spacing: 12

                UTText {
                    text: qsTr("Convert a video file to animated GIF")
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Secondary
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    TextField {
                        id: gifInputPath
                        Layout.fillWidth: true
                        placeholderText: qsTr("Select video file...")
                        readOnly: true
                    }

                    UTButton {
                        text: qsTr("Browse...")
                        onClicked: {
                            // TODO: Open file dialog
                        }
                    }

                    UTButton {
                        text: qsTr("Convert")
                        enabled: gifInputPath.text.length > 0
                        onClicked: {
                            controller.convertToGif(gifInputPath.text)
                        }
                    }
                }
            }
        }

        // Spacer
        Item { Layout.fillHeight: true }

        // === Help Text ===
        UTText {
            Layout.fillWidth: true
            text: qsTr("Tip: Use keyboard shortcuts for quick capture. Press Escape to cancel.")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary
            wrapMode: Text.WordWrap
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

    Connections {
        target: controller

        function onRecordingCompleted(filePath) {
            console.log("Recording saved:", filePath)
        }

        function onErrorOccurred(message) {
            console.error("Recording error:", message)
        }
    }
}
