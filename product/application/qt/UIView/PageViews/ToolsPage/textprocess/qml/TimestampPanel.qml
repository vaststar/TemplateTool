import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    property TimestampToolController controller: TimestampToolController {}

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
            UTGroupBox {
                Layout.fillWidth: true
                title: qsTr("Current Time")

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
            UTGroupBox {
                Layout.fillWidth: true
                title: qsTr("Timestamp → DateTime")

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    // Input row
                    RowLayout {
                        spacing: 8

                        UTTextField {
                            id: tsInput
                            Layout.fillWidth: true
                            placeholderText: qsTr("Enter timestamp...")
                            text: controller.timestampInput
                            onTextChanged: controller.timestampInput = text
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
            UTGroupBox {
                Layout.fillWidth: true
                title: qsTr("DateTime → Timestamp")

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    // Input row
                    RowLayout {
                        spacing: 8

                        UTTextField {
                            id: dtInput
                            Layout.fillWidth: true
                            placeholderText: qsTr("Format: YYYY-MM-DD HH:MM:SS")
                            text: controller.dateTimeInput
                            onTextChanged: controller.dateTimeInput = text
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
