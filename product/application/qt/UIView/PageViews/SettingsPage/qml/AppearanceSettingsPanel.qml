import QtQuick
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

        // Section: Theme
        ColumnLayout {
            spacing: 12
            UTText {
                text: qsTr("Theme")
                fontEnum: UIFontToken.Window_Body_Primary
                colorEnum: UIColorToken.Sidebar_Item_Text
                font.bold: true
            }

            Repeater {
                model: controller ? controller.supportedThemes : []
                delegate: Rectangle {
                    required property var modelData
                    required property int index

                    Layout.fillWidth: true
                    height: 40
                    radius: 6
                    color: controller && controller.currentThemeIndex === index
                        ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
                        : themeArea.containsMouse
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
                            : "transparent"
                    border.width: controller && controller.currentThemeIndex === index ? 1 : 0
                    border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)

                    UTText {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        text: modelData
                        fontEnum: UIFontToken.Body_Text
                        colorEnum: UIColorToken.Sidebar_Item_Text
                    }

                    MouseArea {
                        id: themeArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (controller) controller.setTheme(index)
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true } // spacer
    }
}
