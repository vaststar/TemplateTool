import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: uuidPanel
    property UuidToolController controller: UuidToolController {}

    // Shared style helpers
    readonly property color _inputBg: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background, UIColorState.Normal)
    readonly property color _inputText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text, UIColorState.Normal)
    readonly property color _inputBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Normal)
    readonly property color _inputPlaceholder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder, UIColorState.Normal)
    readonly property color _sectionBg: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border, UIColorState.Normal)
    readonly property color _sectionTitle: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Title, UIColorState.Normal)
    readonly property font _inputFont: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    readonly property font _monoFont: UTComponentUtil.getUIFont(UIFontToken.Monospace_Text)

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
        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Generate UUID")
            font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
            palette.windowText: uuidPanel._sectionTitle
            background: Rectangle {
                y: parent.topPadding - parent.bottomPadding
                width: parent.width
                height: parent.height - parent.topPadding + parent.bottomPadding
                color: uuidPanel._sectionBg
                border.color: uuidPanel._sectionBorder
                border.width: 1
                radius: 4
            }

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

                    TextField {
                        id: uuidOutput
                        Layout.fillWidth: true
                        text: controller.generatedUuid
                        readOnly: true
                        font: uuidPanel._monoFont
                        color: uuidPanel._inputText
                        background: Rectangle {
                            color: uuidPanel._inputBg
                            border.color: uuidOutput.activeFocus
                                ? UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
                                : uuidPanel._inputBorder
                            border.width: 1
                            radius: 4
                        }
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
        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Validate UUID")
            font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
            palette.windowText: uuidPanel._sectionTitle
            background: Rectangle {
                y: parent.topPadding - parent.bottomPadding
                width: parent.width
                height: parent.height - parent.topPadding + parent.bottomPadding
                color: uuidPanel._sectionBg
                border.color: uuidPanel._sectionBorder
                border.width: 1
                radius: 4
            }

            ColumnLayout {
                width: parent.width
                spacing: 8

                RowLayout {
                    spacing: 8

                    TextField {
                        id: validateInput
                        Layout.fillWidth: true
                        placeholderText: qsTr("Enter UUID to validate...")
                        placeholderTextColor: uuidPanel._inputPlaceholder
                        text: controller.validateInput
                        onTextChanged: controller.validateInput = text
                        font: uuidPanel._monoFont
                        color: uuidPanel._inputText
                        background: Rectangle {
                            color: uuidPanel._inputBg
                            border.color: validateInput.activeFocus
                                ? UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
                                : uuidPanel._inputBorder
                            border.width: 1
                            radius: 4
                        }
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
        GroupBox {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: qsTr("Generation History")
            font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
            palette.windowText: uuidPanel._sectionTitle
            background: Rectangle {
                y: parent.topPadding - parent.bottomPadding
                width: parent.width
                height: parent.height - parent.topPadding + parent.bottomPadding
                color: uuidPanel._sectionBg
                border.color: uuidPanel._sectionBorder
                border.width: 1
                radius: 4
            }

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

                ScrollView {
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
