import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    // Panel creates its own controller, initialized by parent via setupController
    property AppearanceSettingsController controller: AppearanceSettingsController {}

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 24

        // Section: Themes
        ColumnLayout {
            spacing: 8

            UTText {
                text: qsTr("Themes")
                fontEnum: UIFontToken.Section_Title
                colorEnum: UIColorToken.Sidebar_Item_Text
            }

            UTText {
                text: qsTr("Choose how you want the app to look.")
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Secondary_Text
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
            activeFocusOnTab: true

            Keys.onReturnPressed: if (controller) controller.setTheme(index)
            Keys.onSpacePressed: if (controller) controller.setTheme(index)

            // Preview card colors per theme index
            // 0=SystemDefault (split), 1=Dark, 2=Light
            readonly property bool isDarkPreview: index === 1
            readonly property bool isLightPreview: index === 2
            readonly property bool isSplitPreview: index === 0

            readonly property color previewBg:        UTComponentUtil.getThemeUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal, !isLightPreview)
            readonly property color previewSidebar:   UTComponentUtil.getThemeUIColor(UIColorToken.Sidebar_Background, UIColorState.Normal, !isLightPreview)
            readonly property color previewContent:   UTComponentUtil.getThemeUIColor(UIColorToken.Content_Background, UIColorState.Normal, !isLightPreview)
            readonly property color previewAccent:    UTComponentUtil.getThemeUIColor(UIColorToken.Button_Primary_Background, UIColorState.Normal, !isLightPreview)
            readonly property color previewTextLine:  UTComponentUtil.getThemeUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal, !isLightPreview)

            readonly property bool isSelected: controller && controller.currentThemeIndex === index

            Column {
                anchors.fill: parent
                spacing: 8

                // Preview thumbnail
                Rectangle {
                    width: 160
                    height: 100
                    radius: 8
                    color: previewBg
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
                            color: UTComponentUtil.getThemeUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal, true)
                            clip: true

                            Row {
                                anchors.fill: parent

                                Rectangle {
                                    width: 16
                                    height: parent.height
                                    color: UTComponentUtil.getThemeUIColor(UIColorToken.Sidebar_Background, UIColorState.Normal, true)
                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 5
                                        Repeater {
                                            model: 3
                                            Rectangle {
                                                required property int index
                                                width: 8; height: 8; radius: 2; color: UTComponentUtil.getThemeUIColor(UIColorToken.Button_Primary_Background, UIColorState.Normal, true); opacity: index === 0 ? 1.0 : 0.3
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    width: parent.width - 18
                                    height: parent.height
                                    color: UTComponentUtil.getThemeUIColor(UIColorToken.Content_Background, UIColorState.Normal, true)
                                    Column {
                                        anchors.left: parent.left; anchors.leftMargin: 6
                                        anchors.top: parent.top; anchors.topMargin: 8
                                        spacing: 5
                                        Rectangle { width: 30; height: 3; radius: 1.5; color: UTComponentUtil.getThemeUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal, true) }
                                        Rectangle { width: 40; height: 2; radius: 1; color: UTComponentUtil.getThemeUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal, true); opacity: 0.5 }
                                        Rectangle { width: 25; height: 2; radius: 1; color: UTComponentUtil.getThemeUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal, true); opacity: 0.5 }
                                        Rectangle { width: 20; height: 6; radius: 3; color: UTComponentUtil.getThemeUIColor(UIColorToken.Button_Primary_Background, UIColorState.Normal, true) }
                                    }
                                }
                            }
                        }

                        // Light half
                        Rectangle {
                            width: parent.width / 2
                            height: parent.height
                            radius: 6
                            color: UTComponentUtil.getThemeUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal, false)
                            clip: true

                            Row {
                                anchors.fill: parent

                                Rectangle {
                                    width: 16
                                    height: parent.height
                                    color: UTComponentUtil.getThemeUIColor(UIColorToken.Sidebar_Background, UIColorState.Normal, false)
                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 5
                                        Repeater {
                                            model: 3
                                            Rectangle {
                                                required property int index
                                                width: 8; height: 8; radius: 2; color: UTComponentUtil.getThemeUIColor(UIColorToken.Button_Primary_Background, UIColorState.Normal, false); opacity: index === 0 ? 1.0 : 0.3
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    width: parent.width - 18
                                    height: parent.height
                                    color: UTComponentUtil.getThemeUIColor(UIColorToken.Content_Background, UIColorState.Normal, false)
                                    Column {
                                        anchors.left: parent.left; anchors.leftMargin: 6
                                        anchors.top: parent.top; anchors.topMargin: 8
                                        spacing: 5
                                        Rectangle { width: 30; height: 3; radius: 1.5; color: UTComponentUtil.getThemeUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal, false) }
                                        Rectangle { width: 40; height: 2; radius: 1; color: UTComponentUtil.getThemeUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal, false); opacity: 0.5 }
                                        Rectangle { width: 25; height: 2; radius: 1; color: UTComponentUtil.getThemeUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal, false); opacity: 0.5 }
                                        Rectangle { width: 20; height: 6; radius: 3; color: UTComponentUtil.getThemeUIColor(UIColorToken.Button_Primary_Background, UIColorState.Normal, false) }
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
                    parent.forceActiveFocus()
                    if (controller) controller.setTheme(index)
                }
            }

            UTFocusItem {
                focusRadius: 8
            }
        }
    }
}
