import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: base64Panel
    property Base64ToolController controller: Base64ToolController {}

    // Shared style helpers
    readonly property color _inputBg: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background, UIColorState.Normal)
    readonly property color _inputText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text, UIColorState.Normal)
    readonly property color _inputBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Normal)
    readonly property color _inputPlaceholder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder, UIColorState.Normal)
    readonly property font _inputFont: UTComponentUtil.getUIFont(UIFontToken.Body_Text)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Title
        UTText {
            text: qsTr("Base64 Encode/Decode")
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Content_Heading
        }

        // Options row
        RowLayout {
            spacing: 16

            UTCheckBox {
                id: urlSafeCheck
                text: qsTr("URL Safe Mode")
                checked: controller.urlSafe
                onToggled: controller.urlSafe = checked
            }
        }

        // Input area
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            UTText {
                text: qsTr("Input")
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Content_Secondary_Text
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 120

                TextArea {
                    id: inputArea
                    text: controller.inputText
                    onTextChanged: controller.inputText = text
                    placeholderText: qsTr("Enter text here...")
                    placeholderTextColor: base64Panel._inputPlaceholder
                    wrapMode: TextArea.Wrap
                    color: base64Panel._inputText
                    font: base64Panel._inputFont
                    background: Rectangle {
                        color: base64Panel._inputBg
                        border.color: inputArea.activeFocus
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
                            : base64Panel._inputBorder
                        border.width: 1
                        radius: 4
                    }
                }
            }
        }

        // Action buttons
        RowLayout {
            spacing: 8

            UTButton {
                text: qsTr("Encode")
                onClicked: controller.encode()
            }
            UTButton {
                text: qsTr("Decode")
                onClicked: controller.decode()
            }
            UTButton {
                text: qsTr("Swap")
                onClicked: controller.swapInputOutput()
            }
            UTButton {
                text: qsTr("Clear")
                onClicked: controller.clearAll()
            }
        }

        // Error message
        UTText {
            visible: controller.errorMessage.length > 0
            text: controller.errorMessage
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Error_Text
        }

        // Output area
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            RowLayout {
                UTText {
                    text: qsTr("Output")
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }
                Item { Layout.fillWidth: true }
                UTButton {
                    text: qsTr("Copy")
                    onClicked: controller.copyOutput()
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                TextArea {
                    id: outputArea
                    text: controller.outputText
                    readOnly: true
                    wrapMode: TextArea.Wrap
                    color: base64Panel._inputText
                    font: base64Panel._inputFont
                    background: Rectangle {
                        color: base64Panel._inputBg
                        border.color: outputArea.activeFocus
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
                            : base64Panel._inputBorder
                        border.width: 1
                        radius: 4
                    }
                }
            }
        }
    }
}
