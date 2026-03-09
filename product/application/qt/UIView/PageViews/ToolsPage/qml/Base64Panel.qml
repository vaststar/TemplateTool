import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: base64Panel
    property Base64ToolController controller: Base64ToolController {}

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Title
        UTText {
            text: "Base64 编解码"
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }

        // Options row
        RowLayout {
            spacing: 16
            
            CheckBox {
                id: urlSafeCheck
                text: "URL 安全模式"
                checked: controller.urlSafe
                onCheckedChanged: controller.urlSafe = checked
            }
        }

        // Input area
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            
            UTText {
                text: "输入"
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
            }
            
            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                
                TextArea {
                    id: inputArea
                    text: controller.inputText
                    onTextChanged: controller.inputText = text
                    placeholderText: "在此输入文本..."
                    wrapMode: TextArea.Wrap
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
                text: "编码"
                onClicked: controller.encode()
            }
            UTButton {
                text: "解码"
                onClicked: controller.decode()
            }
            UTButton {
                text: "交换"
                onClicked: controller.swapInputOutput()
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
