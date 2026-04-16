import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: jsonPanel
    property JsonToolController controller: JsonToolController {}

    // Shared style helpers
    readonly property color _inputBg: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background, UIColorState.Normal)
    readonly property color _inputText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text, UIColorState.Normal)
    readonly property color _inputBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Normal)
    readonly property color _inputPlaceholder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder, UIColorState.Normal)
    readonly property font _inputFont: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    readonly property font _monoFont: UTComponentUtil.getUIFont(UIFontToken.Monospace_Text)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Title
        UTText {
            text: qsTr("JSON Formatter")
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Content_Heading
        }

        // Options row
        RowLayout {
            spacing: 16

            UTText {
                text: qsTr("Indent spaces:")
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Content_Text
            }

            UTSpinBox {
                id: indentSpinBox
                from: 0
                to: 8
                value: controller.indentSize
                onValueChanged: controller.indentSize = value
            }
        }

        // Input area
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            UTText {
                text: qsTr("Input JSON")
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Content_Secondary_Text
            }

            UTScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 150

                UTTextArea {
                    id: inputArea
                    text: controller.inputText
                    onTextChanged: controller.inputText = text
                    placeholderText: qsTr("Enter JSON here...")
                }
            }
        }

        // Action buttons
        RowLayout {
            spacing: 8

            UTButton {
                text: qsTr("Format")
                onClicked: controller.format()
            }
            UTButton {
                text: qsTr("Minify")
                onClicked: controller.minify()
            }
            UTButton {
                text: qsTr("Validate")
                onClicked: controller.validate()
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

            UTScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                UTTextArea {
                    id: outputArea
                    text: controller.outputText
                    readOnly: true
                }
            }
        }
    }
}
