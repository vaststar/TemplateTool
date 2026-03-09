import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: timestampPanel
    property TimestampToolController controller: TimestampToolController {}

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Title
        UTText {
            text: "时间戳转换"
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }

        // Current timestamp section
        RowLayout {
            spacing: 8
            
            UTButton {
                text: "获取当前时间戳"
                onClicked: controller.getCurrentTimestamp()
            }
        }

        // Timestamp to DateTime section
        GroupBox {
            Layout.fillWidth: true
            title: "时间戳 → 日期时间"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                
                RowLayout {
                    spacing: 8
                    
                    TextField {
                        id: timestampInput
                        Layout.fillWidth: true
                        placeholderText: "输入时间戳..."
                        text: controller.timestampInput
                        onTextChanged: controller.timestampInput = text
                    }
                    
                    CheckBox {
                        text: "毫秒"
                        checked: controller.isMilliseconds
                        onCheckedChanged: controller.isMilliseconds = checked
                    }
                    
                    UTButton {
                        text: "转换"
                        onClicked: controller.timestampToDateTime()
                    }
                }
            }
        }

        // DateTime to Timestamp section
        GroupBox {
            Layout.fillWidth: true
            title: "日期时间 → 时间戳"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                
                RowLayout {
                    spacing: 8
                    
                    TextField {
                        id: dateTimeInput
                        Layout.fillWidth: true
                        placeholderText: "格式: YYYY-MM-DD HH:MM:SS"
                        text: controller.dateTimeInput
                        onTextChanged: controller.dateTimeInput = text
                    }
                    
                    UTButton {
                        text: "转换"
                        onClicked: controller.dateTimeToTimestamp()
                    }
                }
            }
        }

        // Result section
        GroupBox {
            Layout.fillWidth: true
            title: "结果"
            
            RowLayout {
                anchors.fill: parent
                spacing: 8
                
                UTText {
                    Layout.fillWidth: true
                    text: controller.resultText || "无结果"
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Sidebar_Item_Text
                    wrapMode: Text.Wrap
                }
                
                UTButton {
                    text: "复制"
                    onClicked: controller.copyResult()
                    enabled: controller.resultText.length > 0
                }
            }
        }

        // Error message
        UTText {
            visible: controller.errorMessage.length > 0
            text: controller.errorMessage
            colorEnum: UIColorToken.Sidebar_Item_Text
            color: "red"
        }

        Item { Layout.fillHeight: true }
    }
}
