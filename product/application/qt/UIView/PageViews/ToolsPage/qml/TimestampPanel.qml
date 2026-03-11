import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: timestampPanel
    property TimestampToolController controller: TimestampToolController {}

    // Shared style helpers
    readonly property color _inputBg: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background, UIColorState.Normal)
    readonly property color _inputText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text, UIColorState.Normal)
    readonly property color _inputBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Normal)
    readonly property color _inputPlaceholder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder, UIColorState.Normal)
    readonly property color _sectionBg: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border, UIColorState.Normal)
    readonly property color _sectionTitle: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Title, UIColorState.Normal)
    readonly property font _inputFont: UTComponentUtil.getUIFont(UIFontToken.Body_Text)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Title
        UTText {
            text: "时间戳转换"
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Content_Heading
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
            font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
            palette.windowText: timestampPanel._sectionTitle
            background: Rectangle {
                y: parent.topPadding - parent.bottomPadding
                width: parent.width
                height: parent.height - parent.topPadding + parent.bottomPadding
                color: timestampPanel._sectionBg
                border.color: timestampPanel._sectionBorder
                border.width: 1
                radius: 4
            }

            ColumnLayout {
                width: parent.width
                spacing: 8

                RowLayout {
                    spacing: 8

                    TextField {
                        id: timestampInput
                        Layout.fillWidth: true
                        placeholderText: "输入时间戳..."
                        placeholderTextColor: timestampPanel._inputPlaceholder
                        text: controller.timestampInput
                        onTextChanged: controller.timestampInput = text
                        color: timestampPanel._inputText
                        font: timestampPanel._inputFont
                        background: Rectangle {
                            color: timestampPanel._inputBg
                            border.color: timestampInput.activeFocus
                                ? UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
                                : timestampPanel._inputBorder
                            border.width: 1
                            radius: 4
                        }
                    }

                    CheckBox {
                        text: "毫秒"
                        checked: controller.isMilliseconds
                        onCheckedChanged: controller.isMilliseconds = checked
                        font: timestampPanel._inputFont
                        palette.windowText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Text, UIColorState.Normal)
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
            font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
            palette.windowText: timestampPanel._sectionTitle
            background: Rectangle {
                y: parent.topPadding - parent.bottomPadding
                width: parent.width
                height: parent.height - parent.topPadding + parent.bottomPadding
                color: timestampPanel._sectionBg
                border.color: timestampPanel._sectionBorder
                border.width: 1
                radius: 4
            }

            ColumnLayout {
                width: parent.width
                spacing: 8

                RowLayout {
                    spacing: 8

                    TextField {
                        id: dateTimeInput
                        Layout.fillWidth: true
                        placeholderText: "格式: YYYY-MM-DD HH:MM:SS"
                        placeholderTextColor: timestampPanel._inputPlaceholder
                        text: controller.dateTimeInput
                        onTextChanged: controller.dateTimeInput = text
                        color: timestampPanel._inputText
                        font: timestampPanel._inputFont
                        background: Rectangle {
                            color: timestampPanel._inputBg
                            border.color: dateTimeInput.activeFocus
                                ? UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
                                : timestampPanel._inputBorder
                            border.width: 1
                            radius: 4
                        }
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
            font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
            palette.windowText: timestampPanel._sectionTitle
            background: Rectangle {
                y: parent.topPadding - parent.bottomPadding
                width: parent.width
                height: parent.height - parent.topPadding + parent.bottomPadding
                color: timestampPanel._sectionBg
                border.color: timestampPanel._sectionBorder
                border.width: 1
                radius: 4
            }

            RowLayout {
                width: parent.width
                spacing: 8

                UTText {
                    Layout.fillWidth: true
                    text: controller.resultText || "无结果"
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Text
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
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Error_Text
        }

        Item { Layout.fillHeight: true }
    }
}
