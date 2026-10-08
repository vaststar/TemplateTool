import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: proxyPanel
    property NetworkProxyController controller: NetworkProxyController {}

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 6

        // ════════════════════════════════════════════
        // Control Bar
        // ════════════════════════════════════════════
        RowLayout {
            Layout.fillWidth: true; spacing: 8

            UTButton {
                text: controller.proxyRunning ? qsTr("■ Stop") : qsTr("▶ Start")
                onClicked: controller.proxyRunning ? controller.stopProxy() : controller.startProxy()
            }

            UTText { text: qsTr("Port:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
            UTTextField {
                id: proxyPortInput; implicitWidth: 60
                fontEnum: UIFontToken.Monospace_Text
                text: controller.proxyPort.toString()
                enabled: !controller.proxyRunning
                validator: IntValidator { bottom: 1024; top: 65535 }
                onEditingFinished: controller.proxyPort = parseInt(text)
            }

            Rectangle { width: 10; height: 10; radius: 5; color: controller.addonConnected ? "#4CAF50" : "#9E9E9E" }
            UTText {
                text: controller.addonConnected ? qsTr("Connected") : qsTr("Disconnected")
                fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
            }

            Item { Layout.fillWidth: true }

            UTText {
                text: qsTr("%1 retained requests").arg(controller.requestCount)
                fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text
            }
            UTButton { text: qsTr("Clear"); onClicked: controller.clearRequests() }
            UTButton { text: qsTr("Export Retained"); enabled: controller.requestCount > 0; onClicked: exportDialog.open() }
        }

        // Status message
        UTText {
            visible: controller.statusMessage.length > 0
            text: controller.statusMessage
            fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            UTText {
                text: controller.rulesManager.configSyncState === "applied" ? qsTr("Rules applied")
                    : controller.rulesManager.configSyncState === "failed" ? qsTr("Rule sync failed")
                    : controller.addonConnected ? qsTr("Syncing rules...") : qsTr("Rules pending sync")
                fontEnum: UIFontToken.Caption_Text
                colorEnum: controller.rulesManager.configSyncState === "failed"
                    ? UIColorToken.Content_Error_Text : UIColorToken.Content_Text
            }
            UTText {
                Layout.fillWidth: true
                visible: controller.rulesManager.configSyncError.length > 0
                text: controller.rulesManager.configSyncError
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Error_Text
                wrapMode: Text.WordWrap
            }
            Item { Layout.fillWidth: true; visible: controller.rulesManager.configSyncError.length === 0 }
            UTButton {
                text: qsTr("Retry Sync")
                visible: controller.rulesManager.configSyncState === "failed"
                enabled: controller.addonConnected
                onClicked: controller.rulesManager.retryConfigSync()
            }
        }

        // ════════════════════════════════════════════
        // TabBar
        // ════════════════════════════════════════════
        UTTabBar {
            id: mainTabBar; Layout.fillWidth: true
            UTTabButton { text: qsTr("📡 Capture") }
            UTTabButton { text: qsTr("📋 Rules") }
            UTTabButton { text: qsTr("⚙ Settings") }
        }

        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: mainTabBar.currentIndex

            NetworkCaptureTab {
                controller: proxyPanel.controller
                onGoToBreakpoints: { mainTabBar.currentIndex = 1; rulesTab.currentRuleSection = 1 }
            }

            NetworkRulesTab {
                id: rulesTab
                controller: proxyPanel.controller
                onTestPattern: function(pattern) { patternTestDialog.openWithPattern(pattern) }
                onShowMockDetail: function(data) { mockDetailDialog.openWithRule(data) }
            }

            NetworkSettingsTab {
                controller: proxyPanel.controller
            }
        }
    }

    // ── Shared Dialogs ──
    PatternTestDialog {
        id: patternTestDialog
        controller: proxyPanel.controller
    }

    MockDetailDialog {
        id: mockDetailDialog
        onEditRule: function(data) {
            rulesTab.loadMockFormData(data)
            mainTabBar.currentIndex = 1
        }
        onTestRulePattern: function(pattern) {
            patternTestDialog.openWithPattern(pattern)
        }
    }

    FileDialog {
        id: exportDialog; title: qsTr("Export Retained Requests"); fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]; onAccepted: controller.exportRequests(selectedFile)
    }
}
