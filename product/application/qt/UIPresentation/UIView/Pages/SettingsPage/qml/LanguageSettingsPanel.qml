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
                id: languageLabel
                text: qsTr("Language")
                fontEnum: UIFontToken.Section_Title
                colorEnum: UIColorToken.Sidebar_Item_Text
            }

            UTText {
                text: qsTr("Choose the display language for the application.")
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Sidebar_Item_Text
                color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal)
            }

            UTComboBox {
                id: languageCombo
                Accessible.name: languageLabel.text + ": " + languageCombo.displayText
                Layout.preferredWidth: 280
                Layout.preferredHeight: 36
                model: controller ? controller.supportedLanguages : []
                currentIndex: controller ? controller.currentLanguageIndex : 0
                onActivated: function(index) {
                    if (controller) controller.setLanguage(index)
                }

                Connections {
                    target: controller
                    function onCurrentLanguageIndexChanged() {
                        languageCombo.currentIndex = controller.currentLanguageIndex
                    }
                }
            }
        }

        Item { Layout.fillHeight: true } // spacer
    }
}
