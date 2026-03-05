import QtQuick
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: settingsPage
    property SettingsPageController controller: SettingsPageController {}

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left nav panel
        SettingsNavPanel {
            id: settingsNav
            controller: settingsPage.controller
            Layout.fillHeight: true
            Layout.preferredWidth: 180
        }

        // Right content panel
        StackLayout {
            id: settingsContent
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: settingsNav.currentNavId

            AppearanceSettingsPanel {
                controller: settingsPage.controller
            }

            LanguageSettingsPanel {
                controller: settingsPage.controller
            }
        }
    }
}
