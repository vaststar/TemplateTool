import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import UTComponent

/**
 * Screen Capture Settings Tab
 */
Item {
    id: root

    required property var controller

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 24

            // === Output Settings ===
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Output Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    // Output directory
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("Save to:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        TextField {
                            id: outputDirField
                            Layout.fillWidth: true
                            text: controller.outputDirectory || controller.getDefaultSavePath()
                            readOnly: true
                        }

                        UTButton {
                            text: qsTr("Browse...")
                            onClicked: folderDialog.open()
                        }
                    }

                    // Image format
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("Image format:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        ComboBox {
                            id: imageFormatCombo
                            model: ["png", "jpg", "bmp"]
                            currentIndex: model.indexOf(controller.imageFormat)
                            onCurrentTextChanged: {
                                if (currentText !== controller.imageFormat) {
                                    controller.imageFormat = currentText
                                }
                            }
                            implicitWidth: 100
                        }

                        UTText {
                            text: qsTr("PNG for quality, JPG for smaller size")
                            fontEnum: UIFontToken.Caption_Text
                            colorEnum: UIColorToken.Content_Secondary
                        }
                    }

                    // Video format
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("Video format:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        ComboBox {
                            id: videoFormatCombo
                            model: ["mp4", "webm", "mov"]
                            currentIndex: model.indexOf(controller.videoFormat)
                            onCurrentTextChanged: {
                                if (currentText !== controller.videoFormat) {
                                    controller.videoFormat = currentText
                                }
                            }
                            implicitWidth: 100
                        }
                    }
                }
            }

            // === Capture Settings ===
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Capture Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    // Delay
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("Default delay:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        ComboBox {
                            id: delayCombo
                            model: [
                                { text: qsTr("None"), value: 0 },
                                { text: qsTr("3 seconds"), value: 3 },
                                { text: qsTr("5 seconds"), value: 5 },
                                { text: qsTr("10 seconds"), value: 10 }
                            ]
                            textRole: "text"
                            valueRole: "value"
                            currentIndex: {
                                for (var i = 0; i < model.length; i++) {
                                    if (model[i].value === controller.delaySeconds) return i
                                }
                                return 0
                            }
                            onCurrentValueChanged: {
                                if (currentValue !== controller.delaySeconds) {
                                    controller.delaySeconds = currentValue
                                }
                            }
                            implicitWidth: 120
                        }
                    }

                    // Timestamp
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("Include timestamp:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        Switch {
                            id: timestampSwitch
                            checked: controller.includeTimestamp
                            onCheckedChanged: {
                                if (checked !== controller.includeTimestamp) {
                                    controller.includeTimestamp = checked
                                }
                            }
                        }

                        UTText {
                            text: qsTr("Add date/time watermark to screenshots")
                            fontEnum: UIFontToken.Caption_Text
                            colorEnum: UIColorToken.Content_Secondary
                        }
                    }
                }
            }

            // === Recording Settings ===
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Recording Settings")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    // FPS
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("Frame rate:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        ComboBox {
                            id: fpsCombo
                            model: [
                                { text: qsTr("15 fps (small size)"), value: 15 },
                                { text: qsTr("30 fps (standard)"), value: 30 },
                                { text: qsTr("60 fps (smooth)"), value: 60 }
                            ]
                            textRole: "text"
                            valueRole: "value"
                            currentIndex: {
                                for (var i = 0; i < model.length; i++) {
                                    if (model[i].value === controller.fps) return i
                                }
                                return 1 // default to 30fps
                            }
                            onCurrentValueChanged: {
                                if (currentValue !== controller.fps) {
                                    controller.fps = currentValue
                                }
                            }
                            implicitWidth: 180
                        }
                    }

                    // FFmpeg path info
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("FFmpeg:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        UTText {
                            text: controller.isFFmpegAvailable()
                                  ? controller.getFFmpegPath()
                                  : qsTr("Not found")
                            fontEnum: UIFontToken.Caption_Text
                            colorEnum: controller.isFFmpegAvailable()
                                       ? UIColorToken.Content_Text
                                       : UIColorToken.Status_Error
                            elide: Text.ElideMiddle
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // Spacer
            Item { Layout.fillHeight: true; Layout.minimumHeight: 20 }
        }
    }

    FolderDialog {
        id: folderDialog
        title: qsTr("Select Output Directory")
        currentFolder: controller.outputDirectory || controller.getDefaultSavePath()
        onAccepted: {
            controller.outputDirectory = selectedFolder
            outputDirField.text = selectedFolder
        }
    }
}
