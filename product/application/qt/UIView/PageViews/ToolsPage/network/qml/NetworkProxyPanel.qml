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

    // ── Theme helpers (shared by control bar & status) ──
    readonly property font  _inputFont:  UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    readonly property font  _monoFont:   Qt.font({family: "Consolas", pixelSize: _inputFont.pixelSize})
    readonly property color _inputBg:    UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background, UIColorState.Normal)
    readonly property color _inputText:  UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text, UIColorState.Normal)
    readonly property color _inputBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Normal)
    readonly property color _accentColor: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)

    // ── Intercepted (paused) flows model (shared by Capture + Rules tabs) ──
    ListModel { id: interceptedFlowsModel }

    Connections {
        target: controller
        function onInterceptedRequest(flowId, detail) {
            interceptedFlowsModel.append({
                flowId: flowId,
                method: detail["method"] || "",
                url: detail["url"] || ""
            })
        }
    }

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
            TextField {
                id: proxyPortInput; implicitWidth: 60
                text: controller.proxyPort.toString()
                enabled: !controller.proxyRunning
                validator: IntValidator { bottom: 1024; top: 65535 }
                onEditingFinished: controller.proxyPort = parseInt(text)
                font: proxyPanel._monoFont; color: proxyPanel._inputText
                background: Rectangle {
                    color: proxyPanel._inputBg
                    border.color: proxyPortInput.activeFocus ? proxyPanel._accentColor : proxyPanel._inputBorder
                    border.width: 1; radius: 3
                }
            }

            Rectangle { width: 10; height: 10; radius: 5; color: controller.addonConnected ? "#4CAF50" : "#9E9E9E" }
            UTText {
                text: controller.addonConnected ? qsTr("Connected") : qsTr("Disconnected")
                fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
            }

            Item { Layout.fillWidth: true }

            UTText {
                text: qsTr("%1 requests").arg(controller.requestCount)
                fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text
            }
            UTButton { text: qsTr("Clear"); onClicked: controller.clearRequests() }
            UTButton { text: qsTr("Export"); enabled: controller.requestCount > 0; onClicked: exportDialog.open() }
        }

        // Status message
        UTText {
            visible: controller.statusMessage.length > 0
            text: controller.statusMessage
            fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
            Layout.fillWidth: true
        }

        // ════════════════════════════════════════════
        // TabBar
        // ════════════════════════════════════════════
        TabBar {
            id: mainTabBar; Layout.fillWidth: true
            TabButton { text: qsTr("📡 Capture") }
            TabButton { text: qsTr("📋 Rules") }
            TabButton { text: qsTr("⚙ Settings") }
        }

        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: mainTabBar.currentIndex

            NetworkCaptureTab {
                controller: proxyPanel.controller
                interceptedFlowsModel: interceptedFlowsModel
                onGoToBreakpoints: { mainTabBar.currentIndex = 1; rulesTab.currentRuleSection = 1 }
            }

            NetworkRulesTab {
                id: rulesTab
                controller: proxyPanel.controller
                interceptedFlowsModel: interceptedFlowsModel
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
        id: exportDialog; title: qsTr("Export Requests"); fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]; onAccepted: controller.exportRequests(selectedFile)
    }
}
