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

    // Section style helpers
    readonly property color _sectionBg: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border, UIColorState.Normal)
    readonly property color _sectionTitle: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Title, UIColorState.Normal)

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: parent.width
            spacing: 24

            // === Output Settings Section ===
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: outputCol.implicitHeight + 52
                color: root._sectionBg
                border.color: root._sectionBorder
                border.width: 1
                radius: 4

                ColumnLayout {
                    id: outputCol
                    anchors {
                        fill: parent
                        topMargin: 36
                        leftMargin: 16
                        rightMargin: 16
                        bottomMargin: 16
                    }
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

                    // Image format
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        UTText {
                            text: qsTr("Image format:")
                            fontEnum: UIFontToken.Body_Text
                            Layout.preferredWidth: 120
                        }

                        UTComboBox {
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

                // Section title
                UTText {
                    x: 12
                    y: 10
                    text: qsTr("Output Settings")
                    fontEnum: UIFontToken.Body_Text_Medium
                    color: root._sectionTitle
                }
            }

            // === Capture Settings Section ===
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: captureCol.implicitHeight + 52
                color: root._sectionBg
                border.color: root._sectionBorder
                border.width: 1
                radius: 4

                ColumnLayout {
                    id: captureCol
                    anchors {
                        fill: parent
                        topMargin: 36
                        leftMargin: 16
                        rightMargin: 16
                        bottomMargin: 16
                    }
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

                        UTComboBox {
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

                        UTSwitch {
                            id: timestampSwitch
                            checked: controller.includeTimestamp
                            onToggled: controller.includeTimestamp = checked
                        }

                        UTText {
                            text: qsTr("Add date/time watermark to screenshots")
                            fontEnum: UIFontToken.Caption_Text
                            colorEnum: UIColorToken.Content_Secondary_Text
                        }
                    }
                }

                // Section title
                UTText {
                    x: 12
                    y: 10
                    text: qsTr("Capture Settings")
                    fontEnum: UIFontToken.Body_Text_Medium
                    color: root._sectionTitle
                }
            }
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
