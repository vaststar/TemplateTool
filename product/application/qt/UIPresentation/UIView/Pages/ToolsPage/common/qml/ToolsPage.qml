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
                    id: panelLoader
                    required property var modelData   // { source: string, preload: bool }
                    required property int index

                    // preload=true: load eagerly. Otherwise wait until the user first selects this
                    // panel; once shown, stay loaded so subsequent selections do not rebuild it.
                    property bool everShown: modelData.preload || index === contentStack.currentIndex
                    Connections {
                        target: contentStack
                        function onCurrentIndexChanged() {
                            if (index === contentStack.currentIndex)
                                panelLoader.everShown = true
                        }
                    }

                    active: everShown
                    source: active ? modelData.source : ""

                    onLoaded: {
                        if (item && item.controller)
                            toolsPage.controller.setupController(item.controller)
                    }
                }
            }
        }
    }
}
