import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
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
            text: "UUID 生成"
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }

        // Generate section
        GroupBox {
            Layout.fillWidth: true
            title: "生成 UUID"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                
                RowLayout {
                    spacing: 8
                    
                    UTButton {
                        text: "生成"
                        onClicked: controller.generate()
                    }
                    
                    UTButton {
                        text: "批量生成 (5个)"
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
                        font.family: "monospace"
                    }
                    
                    UTButton {
                        text: "复制"
                        onClicked: controller.copyUuid()
                        enabled: controller.generatedUuid.length > 0
                    }
                }
            }
        }

        // Validate section
        GroupBox {
            Layout.fillWidth: true
            title: "校验 UUID"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                
                RowLayout {
                    spacing: 8
                    
                    TextField {
                        id: validateInput
                        Layout.fillWidth: true
                        placeholderText: "输入要校验的 UUID..."
                        text: controller.validateInput
                        onTextChanged: controller.validateInput = text
                        font.family: "monospace"
                    }
                    
                    UTButton {
                        text: "校验"
                        onClicked: controller.validate()
                    }
                }
                
                UTText {
                    visible: controller.validateResult.length > 0
                    text: controller.validateResult
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Sidebar_Item_Text
                }
            }
        }

        // History section
        GroupBox {
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: "生成历史"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                
                RowLayout {
                    UTButton {
                        text: "复制全部"
                        onClicked: controller.copyHistory()
                        enabled: controller.uuidHistory.length > 0
                    }
                    UTButton {
                        text: "清空历史"
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
                                : UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)
                            
                            UTText {
                                anchors.left: parent.left
                                anchors.leftMargin: 8
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData
                                fontEnum: UIFontToken.Body_Text
                                colorEnum: UIColorToken.Sidebar_Item_Text
                                font.family: "monospace"
                            }
                        }
                    }
                }
            }
        }
    }
}
