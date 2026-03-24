import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import UTComponent
import UIResourceLoader 1.0

/**
 * Recording Settings Tab - Recording-specific settings
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
                                return 1
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
                            text: controller.ffmpegAvailable
                                  ? controller.ffmpegPath
                                  : qsTr("Not found")
                            fontEnum: UIFontToken.Caption_Text
                            colorEnum: controller.ffmpegAvailable
                                       ? UIColorToken.Content_Text
                                       : UIColorToken.Content_Error_Text
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
        currentFolder: "file://" + (controller.outputDirectory || controller.getDefaultSavePath())
        onAccepted: {
            controller.outputDirectory = selectedFolder
            outputDirField.text = controller.outputDirectory
        }
    }
}
