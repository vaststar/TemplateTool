import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    property TimestampToolController controller: TimestampToolController {}

    // ── shared style helpers ──
    readonly property color _inputBg:          UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background,  UIColorState.Normal)
    readonly property color _inputText:        UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text,        UIColorState.Normal)
    readonly property color _inputBorder:      UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,      UIColorState.Normal)
    readonly property color _inputBorderFocus: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,      UIColorState.Focused)
    readonly property color _inputPlaceholder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder, UIColorState.Normal)
    readonly property color _sectionBg:        UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder:    UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border,    UIColorState.Normal)
    readonly property color _sectionTitle:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Title,     UIColorState.Normal)
    readonly property color _textColor:        UTComponentUtil.getPlainUIColor(UIColorToken.Content_Text,              UIColorState.Normal)
    readonly property font  _inputFont:        UTComponentUtil.getUIFont(UIFontToken.Body_Text)

    // ── reusable section background ──
    component SectionBackground: Rectangle {
        y:      parent.topPadding - parent.bottomPadding
        width:  parent.width
        height: parent.height - parent.topPadding + parent.bottomPadding
        color:  root._sectionBg
        border.color: root._sectionBorder
        border.width: 1
        radius: 4
    }

    // ── reusable read-only result row: label  value  [copy] ──
    component ResultRow: RowLayout {
        property string label
        property string value
        spacing: 8

        UTText {
            text: parent.label
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Section_Title
            Layout.preferredWidth: 120
        }
        UTText {
            Layout.fillWidth: true
            text: parent.value || "—"
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Text
            wrapMode: Text.Wrap
        }
        UTButton {
            text: qsTr("Copy")
            visible: parent.value.length > 0
            onClicked: controller.copyText(parent.value)
        }
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: 16
        contentWidth: width
        contentHeight: mainColumn.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: mainColumn
            width: parent.width
            spacing: 16

            // ── Title ──
            UTText {
                text: qsTr("Timestamp Converter")
                fontEnum: UIFontToken.Heading_Text
                colorEnum: UIColorToken.Content_Heading
            }

            // ────────────────────────────────────────────
            // Section 1: Live Clock
            // ────────────────────────────────────────────
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Current Time")
                font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
                palette.windowText: root._sectionTitle
                background: SectionBackground {}

                ColumnLayout {
                    width: parent.width
                    spacing: 6

                    // Big timestamp display + copy button
                    RowLayout {
                        spacing: 8
                        UTText {
                            text: controller.liveTimestamp
                            fontEnum: UIFontToken.Heading_Text
                            colorEnum: UIColorToken.Content_Heading
                        }
                        UTButton {
                            text: qsTr("Copy")
                            onClicked: controller.copyLiveTimestamp()
                        }
                    }

                    ResultRow { label: qsTr("UTC");        value: controller.liveUtcDateTime }
                    ResultRow { label: qsTr("Local Time"); value: controller.liveLocalDateTime }
                    ResultRow { label: qsTr("Timezone");   value: controller.liveTimezone }
                }
            }

            // ────────────────────────────────────────────
            // Section 2: Timestamp → DateTime
            // ────────────────────────────────────────────
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Timestamp → DateTime")
                font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
                palette.windowText: root._sectionTitle
                background: SectionBackground {}

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    // Input row
                    RowLayout {
                        spacing: 8

                        TextField {
                            id: tsInput
                            Layout.fillWidth: true
                            placeholderText: qsTr("Enter timestamp...")
                            placeholderTextColor: root._inputPlaceholder
                            text: controller.timestampInput
                            onTextChanged: controller.timestampInput = text
                            color: root._inputText
                            font:  root._inputFont
                            background: Rectangle {
                                color: root._inputBg
                                border.color: tsInput.activeFocus ? root._inputBorderFocus : root._inputBorder
                                border.width: 1; radius: 4
                            }
                        }

                        UTCheckBox {
                            text: qsTr("Milliseconds")
                            checked: controller.isMilliseconds
                            onToggled: controller.isMilliseconds = checked
                        }

                        UTButton {
                            text: qsTr("Convert")
                            onClicked: controller.timestampToDateTime()
                        }
                    }

                    // Results
                    ResultRow { label: qsTr("UTC");           value: controller.tsResultUtc }
                    ResultRow { label: qsTr("Local Time");    value: controller.tsResultLocal }
                    ResultRow { label: qsTr("Relative Time"); value: controller.tsResultRelative }
                }
            }

            // ────────────────────────────────────────────
            // Section 3: DateTime → Timestamp
            // ────────────────────────────────────────────
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("DateTime → Timestamp")
                font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
                palette.windowText: root._sectionTitle
                background: SectionBackground {}

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    // Input row
                    RowLayout {
                        spacing: 8

                        TextField {
                            id: dtInput
                            Layout.fillWidth: true
                            placeholderText: qsTr("Format: YYYY-MM-DD HH:MM:SS")
                            placeholderTextColor: root._inputPlaceholder
                            text: controller.dateTimeInput
                            onTextChanged: controller.dateTimeInput = text
                            color: root._inputText
                            font:  root._inputFont
                            background: Rectangle {
                                color: root._inputBg
                                border.color: dtInput.activeFocus ? root._inputBorderFocus : root._inputBorder
                                border.width: 1; radius: 4
                            }
                        }

                        UTComboBox {
                            Layout.preferredWidth: 180
                            model: controller.timezoneModel
                            currentIndex: controller.dtTimezoneIndex
                            onActivated: function(index) { controller.dtTimezoneIndex = index }
                        }

                        UTButton {
                            text: qsTr("Convert")
                            onClicked: controller.dateTimeToTimestamp()
                        }
                    }

                    // Results
                    ResultRow { label: qsTr("Seconds");      value: controller.dtResultSeconds }
                    ResultRow { label: qsTr("Milliseconds"); value: controller.dtResultMillis }
                }
            }

            // ── Error message ──
            UTText {
                visible: controller.errorMessage.length > 0
                text: controller.errorMessage
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Content_Error_Text
            }

            Item { Layout.fillHeight: true }
        }
    }
}
