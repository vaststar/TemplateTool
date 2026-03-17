import QtQuick
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: toolsPage
    property ToolsPageController controller: ToolsPageController {}

    // ── Panel cache: lazy-load on first visit, never destroy ──
    // Tracks which QML panels have been activated (visited at least once)
    property var _activatedPanels: ({})

    // All possible panel QML files
    readonly property var _panelList: [
        "Base64Panel.qml",
        "JsonPanel.qml",
        "TimestampPanel.qml",
        "UuidPanel.qml",
        "NetworkProxyPanel.qml"
    ]

    // When current panel changes, mark it as activated
    Connections {
        target: controller
        function onCurrentPanelQmlChanged() {
            var qml = controller.currentPanelQml
            if (qml && !toolsPage._activatedPanels[qml]) {
                var copy = toolsPage._activatedPanels
                copy[qml] = true
                toolsPage._activatedPanels = copy
            }
        }
    }

    Component.onCompleted: {
        // Activate the initial panel
        var qml = controller.currentPanelQml
        if (qml) {
            var copy = _activatedPanels
            copy[qml] = true
            _activatedPanels = copy
        }
    }

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

        // Right content panel: one Loader per panel type, cached
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Repeater {
                model: toolsPage._panelList

                Loader {
                    anchors.fill: parent
                    // Only load once the panel has been visited
                    active: !!toolsPage._activatedPanels[modelData]
                    // Only show the current panel
                    visible: controller.currentPanelQml === modelData
                    source: modelData

                    onLoaded: {
                        // Initialize panel controller via setupController (once)
                        if (item && "controller" in item && item.controller) {
                            toolsPage.controller.setupController(item.controller)
                        }
                    }
                }
            }
        }
    }
}
