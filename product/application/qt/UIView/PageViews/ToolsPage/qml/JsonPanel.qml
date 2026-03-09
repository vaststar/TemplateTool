import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: jsonPanel
    property JsonToolController controller: JsonToolController {}

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Title
        UTText {
            text: "JSON 格式化"
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }

        // Options row
        RowLayout {
            spacing: 16
            
            UTText {
                text: "缩进空格数："
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
            }
            
            SpinBox {
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
                text: "输入 JSON"
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
            }
            
            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 150
                
                TextArea {
                    id: inputArea
                    text: controller.inputText
                    onTextChanged: controller.inputText = text
                    placeholderText: "在此输入 JSON..."
                    wrapMode: TextArea.Wrap
                    font.family: "monospace"
                    background: Rectangle {
                        color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)
                        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Border, UIColorState.Normal)
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
            colorEnum: UIColorToken.Sidebar_Item_Text
            color: "red"
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
                    colorEnum: UIColorToken.Sidebar_Item_Text
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
                    text: controller.outputText
                    readOnly: true
                    wrapMode: TextArea.Wrap
                    font.family: "monospace"
                    background: Rectangle {
                        color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)
                        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Border, UIColorState.Normal)
                        border.width: 1
                        radius: 4
                    }
                }
            }
        }
    }
}
