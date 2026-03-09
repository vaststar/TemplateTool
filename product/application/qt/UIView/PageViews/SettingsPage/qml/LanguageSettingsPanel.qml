import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    // Panel creates its own controller, initialized by parent via setupController
    property LanguageSettingsController controller: LanguageSettingsController {}

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 24

        // Section: Language
        ColumnLayout {
            spacing: 12

            UTText {
                text: qsTr("Language")
                fontEnum: UIFontToken.Window_Body_Primary
                colorEnum: UIColorToken.Sidebar_Item_Text
                font.bold: true
            }

            UTText {
                text: qsTr("Choose the display language for the application.")
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
                color: Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal), 0.6)
            }

            ComboBox {
                id: languageCombo
                Layout.preferredWidth: 280
                Layout.preferredHeight: 36
                model: controller ? controller.supportedLanguages : []
                currentIndex: controller ? controller.currentLanguageIndex : 0

                onActivated: function(index) {
                    if (controller) controller.setLanguage(index)
                }

                // Sync when controller changes externally
                Connections {
                    target: controller
                    function onCurrentLanguageIndexChanged() {
                        languageCombo.currentIndex = controller.currentLanguageIndex
                    }
                }

                background: Rectangle {
                    radius: 6
                    color: languageCombo.pressed
                        ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
                        : languageCombo.hovered
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
                            : UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)
                    border.width: 1
                    border.color: languageCombo.activeFocus
                        ? UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)
                        : Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Border, UIColorState.Normal), 0.5)
                }

                contentItem: Text {
                    leftPadding: 12
                    rightPadding: languageCombo.indicator.width + languageCombo.spacing
                    text: languageCombo.displayText
                    font.pixelSize: 13
                    color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                indicator: Text {
                    x: languageCombo.width - width - 12
                    y: (languageCombo.height - height) / 2
                    text: "\u25BE"  // ▾
                    font.pixelSize: 14
                    color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)
                }

                popup: Popup {
                    y: languageCombo.height + 4
                    width: languageCombo.width
                    padding: 4

                    background: Rectangle {
                        radius: 6
                        color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)
                        border.width: 1
                        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Border, UIColorState.Normal)
                        layer.enabled: true
                    }

                    contentItem: ListView {
                        implicitHeight: contentHeight
                        model: languageCombo.popup.visible ? languageCombo.delegateModel : null
                        clip: true
                        currentIndex: languageCombo.highlightedIndex
                        ScrollIndicator.vertical: ScrollIndicator {}
                    }
                }

                delegate: ItemDelegate {
                    required property var modelData
                    required property int index

                    width: languageCombo.width - 8
                    height: 32
                    leftPadding: 12

                    contentItem: Text {
                        text: modelData
                        font.pixelSize: 13
                        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Text, UIColorState.Normal)
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    background: Rectangle {
                        radius: 4
                        color: parent.hovered
                            ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
                            : languageCombo.currentIndex === index
                                ? UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
                                : "transparent"
                    }

                    highlighted: languageCombo.highlightedIndex === index
                }
            }
        }

        Item { Layout.fillHeight: true } // spacer
    }
}
