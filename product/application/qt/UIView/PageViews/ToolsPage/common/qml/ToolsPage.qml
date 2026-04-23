import QtQuick
import QtQuick.Controls
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

        // Right content panel — StackLayout driven by currentPanelType
        StackLayout {
            id: contentStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: controller.panelRegistry ? controller.panelRegistry.indexOfPanel(controller.currentPanelType) : -1

            Repeater {
                model: controller.initialized && controller.panelRegistry ? controller.panelRegistry.entries : []

                Loader {
                    required property string modelData
                    required property int index
                    source: modelData
                    onLoaded: {
                        if (item && item.controller)
                            toolsPage.controller.setupController(item.controller)
                    }
                }
            }
        }
    }
}
