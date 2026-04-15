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
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: parent.width
            spacing: 24

            // === Output Settings Section ===
            UTGroupBox {
                Layout.fillWidth: true
                title: qsTr("Output Settings")

                ColumnLayout {
                    width: parent.width
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

                        UTTextField {
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

                        UTComboBox {
                            id: videoFormatCombo
                            model: ["mp4", "webm", "mov", "gif"]
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

            // === Recording Settings Section ===
            UTGroupBox {
                Layout.fillWidth: true
                title: qsTr("Recording Settings")

                ColumnLayout {
                    width: parent.width
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

                        UTComboBox {
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

                    // Microphone enable + device selection
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("Microphone:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        UTCheckBox {
                            id: micCheckBox
                            checked: controller.enableMicrophone
                            onToggled: controller.enableMicrophone = checked
                        }

                        UTComboBox {
                            id: micDeviceCombo
                            visible: controller.enableMicrophone
                            enabled: controller.enableMicrophone
                            model: controller.micDevices
                            textRole: "displayName"
                            valueRole: "id"
                            currentIndex: {
                                for (var i = 0; i < controller.micDevices.length; i++) {
                                    if (controller.micDevices[i].id === controller.selectedMicDevice) return i
                                }
                                return 0
                            }
                            onCurrentValueChanged: {
                                if (currentValue && currentValue !== controller.selectedMicDevice) {
                                    controller.selectedMicDevice = currentValue
                                }
                            }
                            implicitWidth: 250
                        }
                    }

                    // Microphone permission warning (macOS)
                    UTText {
                        visible: controller.enableMicrophone && !controller.micPermissionGranted
                        text: qsTr("⚠ Microphone permission required. Go to System Settings > Privacy & Security > Microphone.")
                        fontEnum: UIFontToken.Caption_Text
                        colorEnum: UIColorToken.Content_Error_Text
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    // System audio enable + device selection
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("System Audio:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        UTCheckBox {
                            id: sysAudioCheckBox
                            checked: controller.enableSystemAudio
                            onToggled: controller.enableSystemAudio = checked
                        }

                        UTComboBox {
                            id: sysAudioDeviceCombo
                            visible: controller.enableSystemAudio
                            enabled: controller.enableSystemAudio
                            model: controller.systemAudioDevices
                            textRole: "displayName"
                            valueRole: "id"
                            currentIndex: {
                                for (var i = 0; i < controller.systemAudioDevices.length; i++) {
                                    if (controller.systemAudioDevices[i].id === controller.selectedSystemAudioDevice) return i
                                }
                                return 0
                            }
                            onCurrentValueChanged: {
                                if (currentValue && currentValue !== controller.selectedSystemAudioDevice) {
                                    controller.selectedSystemAudioDevice = currentValue
                                }
                            }
                            implicitWidth: 250
                        }
                    }

                    // System audio device warning
                    UTText {
                        visible: controller.enableSystemAudio && controller.systemAudioDevices.length === 0
                        text: qsTr("⚠ No system audio device found. Install a virtual audio device (e.g. BlackHole on macOS, Stereo Mix on Windows).")
                        fontEnum: UIFontToken.Caption_Text
                        colorEnum: UIColorToken.Content_Error_Text
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
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
        }
    }

    FolderDialog {
        id: folderDialog
        title: qsTr("Select Output Directory")
        currentFolder: controller.recordingsFolderUrl
        onAccepted: {
            controller.outputDirectory = selectedFolder
            outputDirField.text = controller.outputDirectory
        }
    }
}
