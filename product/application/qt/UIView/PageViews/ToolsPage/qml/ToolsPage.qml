import QtQuick
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: toolsPage
    property ToolsPageController controller: ToolsPageController {}



    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left nav panel
        ToolsNavPanel {
            id: toolsNav
            controller: toolsPage.controller
            Layout.fillHeight: true
            Layout.preferredWidth: 200
        }

        // Right content panel (dynamic loader)
        Loader {
            id: contentLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: controller.currentPanelQml

            onLoaded: {
                // Initialize panel controller via setupController
                if (item && "controller" in item && item.controller) {
                    toolsPage.controller.setupController(item.controller)
                }
            }
        }
    }
}
