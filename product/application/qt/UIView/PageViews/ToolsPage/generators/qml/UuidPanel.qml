import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: uuidPanel
    property UuidToolController controller: UuidToolController {}

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Title
        UTText {
            text: qsTr("UUID Generator")
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Content_Heading
        }

        // Generate section
        UTGroupBox {
            Layout.fillWidth: true
            title: qsTr("Generate UUID")

            ColumnLayout {
                width: parent.width
                spacing: 8

                RowLayout {
                    spacing: 8

                    UTButton {
                        text: qsTr("Generate")
                        onClicked: controller.generate()
                    }

                    UTButton {
                        text: qsTr("Batch Generate (5)")
                        onClicked: controller.generateMultiple(5)
                    }
                }

                RowLayout {
                    spacing: 8

                    UTTextField {
                        id: uuidOutput
                        Layout.fillWidth: true
                        text: controller.generatedUuid
                        readOnly: true
                        fontEnum: UIFontToken.Monospace_Text
                    }

                    UTButton {
                        text: qsTr("Copy")
                        onClicked: controller.copyUuid()
                        enabled: controller.generatedUuid.length > 0
                    }
                }
            }
        }

        // Validate section
        UTGroupBox {
            Layout.fillWidth: true
            title: qsTr("Validate UUID")

            ColumnLayout {
                width: parent.width
                spacing: 8

                RowLayout {
                    spacing: 8

                    UTTextField {
                        id: validateInput
                        Layout.fillWidth: true
                        placeholderText: qsTr("Enter UUID to validate...")
                        text: controller.validateInput
                        onTextChanged: controller.validateInput = text
                        fontEnum: UIFontToken.Monospace_Text
                    }

                    UTButton {
                        text: qsTr("Validate")
                        onClicked: controller.validate()
                    }
                }

                UTText {
                    visible: controller.validateResult.length > 0
                    text: controller.validateResult
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Text
                }
            }
        }

        // History section
        UTGroupBox {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: qsTr("Generation History")

            ColumnLayout {
                width: parent.width
                height: parent.height
                spacing: 8

                RowLayout {
                    UTButton {
                        text: qsTr("Copy All")
                        onClicked: controller.copyHistory()
                        enabled: controller.uuidHistory.length > 0
                    }
                    UTButton {
                        text: qsTr("Clear History")
                        onClicked: controller.clearHistory()
                        enabled: controller.uuidHistory.length > 0
                    }
                }

                UTScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ListView {
                        id: historyList
                        model: controller.uuidHistory
                        clip: true

                        delegate: Rectangle {
                            width: historyList.width
                            height: 28
                            color: index % 2 === 0 ? "transparent"
                                : UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)

                            UTText {
                                anchors.left: parent.left
                                anchors.leftMargin: 8
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData
                                fontEnum: UIFontToken.Monospace_Text
                                colorEnum: UIColorToken.Content_Text
                            }
                        }
                    }
                }
            }
        }
    }
}
