import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import UTComponent
import UIResourceLoader 1.0

/**
 * Screenshot Settings Tab - Screenshot-specific settings
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
                            colorEnum: UIColorToken.Content_Secondary_Text
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
                            colorEnum: UIColorToken.Content_Secondary_Text
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
        currentFolder: controller.screenshotsFolderUrl
        onAccepted: {
            controller.outputDirectory = selectedFolder
            outputDirField.text = controller.outputDirectory
        }
    }
}
