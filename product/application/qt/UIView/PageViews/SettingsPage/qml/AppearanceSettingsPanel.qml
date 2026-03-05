import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    property SettingsPageController controller

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 24

        // Section: Themes
        ColumnLayout {
            spacing: 8

            UTText {
                text: qsTr("Themes")
                fontEnum: UIFontToken.Window_Body_Primary
                colorEnum: UIColorToken.Sidebar_Item_Text
                font.bold: true
            }

            UTText {
                text: qsTr("Choose how you want the app to look.")
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
                opacity: 0.6
            }

            // Theme cards grid
            Grid {
                columns: 3
                spacing: 16
                Layout.topMargin: 8

                Repeater {
                    model: controller ? controller.supportedThemes : []
                    delegate: themeCardComponent
                }
            }
        }

        Item { Layout.fillHeight: true } // spacer
    }

    // ── Theme card component ──
    Component {
        id: themeCardComponent

        Item {
            required property var modelData
            required property int index

            width: 160
            height: 148

            // Preview card colors per theme index
            // 0=SystemDefault (split), 1=Dark, 2=Light
            readonly property bool isDarkPreview: index === 1
            readonly property bool isLightPreview: index === 2
            readonly property bool isSplitPreview: index === 0

            readonly property color previewBg:        isLightPreview ? "#f9fafb" : "#1c1c1e"
            readonly property color previewSidebar:    isLightPreview ? "#f3f4f6" : "#27272a"
            readonly property color previewContent:    isLightPreview ? "#e5e7eb" : "#3f3f46"
            readonly property color previewAccent:     isLightPreview ? "#2563eb" : "#60a5fa"
            readonly property color previewTextLine:   isLightPreview ? "#d1d5db" : "#52525b"

            readonly property bool isSelected: controller && controller.currentThemeIndex === index

            Column {
                anchors.fill: parent
                spacing: 8

                // Preview thumbnail
                Rectangle {
                    width: 160
                    height: 100
                    radius: 8
                    color: isSplitPreview ? "#1c1c1e" : previewBg
                    border.width: isSelected ? 2 : 1
                    border.color: isSelected
                        ? UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)
                        : UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Border, UIColorState.Normal)
                    clip: true

                    // ── Split preview (SystemDefault): left dark, right light ──
                    Row {
                        anchors.fill: parent
                        anchors.margins: 2
                        visible: isSplitPreview

                        // Dark half
                        Rectangle {
                            width: parent.width / 2
                            height: parent.height
                            radius: 6
                            color: "#1c1c1e"
                            clip: true

                            Row {
                                anchors.fill: parent

                                Rectangle {
                                    width: 16
                                    height: parent.height
                                    color: "#27272a"
                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 5
                                        Repeater {
                                            model: 3
                                            Rectangle {
                                                required property int index
                                                width: 8; height: 8; radius: 2; color: "#60a5fa"; opacity: index === 0 ? 1.0 : 0.3
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    width: parent.width - 18
                                    height: parent.height
                                    color: "#3f3f46"
                                    Column {
                                        anchors.left: parent.left; anchors.leftMargin: 6
                                        anchors.top: parent.top; anchors.topMargin: 8
                                        spacing: 5
                                        Rectangle { width: 30; height: 3; radius: 1.5; color: "#52525b" }
                                        Rectangle { width: 40; height: 2; radius: 1; color: "#52525b"; opacity: 0.5 }
                                        Rectangle { width: 25; height: 2; radius: 1; color: "#52525b"; opacity: 0.5 }
                                        Rectangle { width: 20; height: 6; radius: 3; color: "#60a5fa" }
                                    }
                                }
                            }
                        }

                        // Light half
                        Rectangle {
                            width: parent.width / 2
                            height: parent.height
                            radius: 6
                            color: "#f9fafb"
                            clip: true

                            Row {
                                anchors.fill: parent

                                Rectangle {
                                    width: 16
                                    height: parent.height
                                    color: "#f3f4f6"
                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 5
                                        Repeater {
                                            model: 3
                                            Rectangle {
                                                required property int index
                                                width: 8; height: 8; radius: 2; color: "#2563eb"; opacity: index === 0 ? 1.0 : 0.3
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    width: parent.width - 18
                                    height: parent.height
                                    color: "#e5e7eb"
                                    Column {
                                        anchors.left: parent.left; anchors.leftMargin: 6
                                        anchors.top: parent.top; anchors.topMargin: 8
                                        spacing: 5
                                        Rectangle { width: 30; height: 3; radius: 1.5; color: "#d1d5db" }
                                        Rectangle { width: 40; height: 2; radius: 1; color: "#d1d5db"; opacity: 0.5 }
                                        Rectangle { width: 25; height: 2; radius: 1; color: "#d1d5db"; opacity: 0.5 }
                                        Rectangle { width: 20; height: 6; radius: 3; color: "#2563eb" }
                                    }
                                }
                            }
                        }
                    }

                    // ── Normal preview (Dark / Light) ──
                    Row {
                        anchors.fill: parent
                        anchors.margins: 2
                        visible: !isSplitPreview

                        // Mini sidebar
                        Rectangle {
                            width: 32
                            height: parent.height
                            radius: 6
                            color: previewSidebar

                            Column {
                                anchors.centerIn: parent
                                spacing: 6
                                Repeater {
                                    model: 4
                                    Rectangle {
                                        required property int index
                                        width: 14
                                        height: 14
                                        radius: 3
                                        color: index === 0 ? previewAccent : previewTextLine
                                        opacity: index === 0 ? 1.0 : 0.4
                                    }
                                }
                            }
                        }

                        // Mini content area
                        Rectangle {
                            width: parent.width - 34
                            height: parent.height
                            radius: 6
                            color: previewContent

                            Column {
                                anchors.left: parent.left
                                anchors.leftMargin: 8
                                anchors.top: parent.top
                                anchors.topMargin: 10
                                spacing: 6

                                // Titlebar line
                                Rectangle {
                                    width: 60
                                    height: 4
                                    radius: 2
                                    color: previewBg
                                }

                                // Content lines
                                Repeater {
                                    model: 3
                                    Rectangle {
                                        width: 40 + Math.random() * 40
                                        height: 3
                                        radius: 1.5
                                        color: previewTextLine
                                        opacity: 0.5
                                    }
                                }

                                // Accent button
                                Rectangle {
                                    width: 30
                                    height: 8
                                    radius: 4
                                    color: previewAccent
                                }
                            }
                        }
                    }
                }

                // Radio + label row
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 6

                    // Radio indicator
                    Rectangle {
                        width: 18
                        height: 18
                        radius: 9
                        anchors.verticalCenter: parent.verticalCenter
                        color: "transparent"
                        border.width: 2
                        border.color: isSelected
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)
                            : UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)

                        Rectangle {
                            anchors.centerIn: parent
                            width: 10
                            height: 10
                            radius: 5
                            color: UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)
                            visible: isSelected
                        }
                    }

                    UTText {
                        text: modelData
                        fontEnum: UIFontToken.Body_Text
                        colorEnum: UIColorToken.Sidebar_Item_Text
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (controller) controller.setTheme(index)
                }
            }
        }
    }
}
