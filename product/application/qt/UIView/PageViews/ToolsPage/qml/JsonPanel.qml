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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Title
        UTText {
            text: "JSON 格式化"
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Content_Heading
        }

        // Options row
        RowLayout {
            spacing: 16

            UTText {
                text: "缩进空格数："
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Content_Text
            }

            SpinBox {
                id: indentSpinBox
                from: 0
                to: 8
                value: controller.indentSize
                onValueChanged: controller.indentSize = value
                font: jsonPanel._inputFont
                palette.buttonText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Text, UIColorState.Normal)
            }
        }

        // Input area
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            UTText {
                text: "输入 JSON"
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Content_Secondary_Text
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 150

                TextArea {
                    id: inputArea
                    text: controller.inputText
                    onTextChanged: controller.inputText = text
                    placeholderText: "在此输入 JSON..."
                    placeholderTextColor: jsonPanel._inputPlaceholder
                    wrapMode: TextArea.Wrap
                    font.family: "Consolas"
                    font.pixelSize: jsonPanel._inputFont.pixelSize
                    color: jsonPanel._inputText
                    background: Rectangle {
                        color: jsonPanel._inputBg
                        border.color: inputArea.activeFocus
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
                            : jsonPanel._inputBorder
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
                text: "格式化"
                onClicked: controller.format()
            }
            UTButton {
                text: "压缩"
                onClicked: controller.minify()
            }
            UTButton {
                text: "校验"
                onClicked: controller.validate()
            }
            UTButton {
                text: "清空"
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
                    text: "输出"
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }
                Item { Layout.fillWidth: true }
                UTButton {
                    text: "复制"
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
                    font.family: "Consolas"
                    font.pixelSize: jsonPanel._inputFont.pixelSize
                    color: jsonPanel._inputText
                    background: Rectangle {
                        color: jsonPanel._inputBg
                        border.color: outputArea.activeFocus
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
                            : jsonPanel._inputBorder
                        border.width: 1
                        radius: 4
                    }
                }
            }
        }
    }
}
