import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root

    required property NetworkProxyController controller
    required property var interceptedFlowsModel

    signal testPattern(string pattern)
    signal showMockDetail(var ruleData)

    property alias currentRuleSection: rulesNavRepeater.currentIndex

    function loadMockFormData(data) {
        mockUrl.text     = data.url_pattern || ""
        mockStatus.text  = String(data.status_code || 200)
        mockCt.text      = data.content_type || "application/json"
        mockBody.text    = data.body || ""
        mockHeaders.text = data.headers || ""
        rulesNavRepeater.currentIndex = 0
    }

    // ── Theme ──
    readonly property color _sectionBg:      UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder:   UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border,     UIColorState.Normal)
    readonly property color _sectionTitle:    UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Title,      UIColorState.Normal)
    readonly property color _headerBg:        Qt.darker(_sectionBg, 1.08)
    readonly property color _headerHover:     Qt.darker(_sectionBg, 1.15)
    readonly property color _inputBg:         UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background,   UIColorState.Normal)
    readonly property color _inputBorder:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Normal)
    readonly property color _accentColor:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Focused)

    RowLayout {
        anchors.fill: parent; spacing: 0

        // ── Left nav ──
        Rectangle {
            Layout.preferredWidth: 140; Layout.fillHeight: true
            color: root._headerBg; radius: 4
            Column {
                anchors.fill: parent; anchors.margins: 4; spacing: 2
                Repeater {
                    id: rulesNavRepeater
                    model: [
                        {icon: "🎭", label: qsTr("Mock Response")},
                        {icon: "🔴", label: qsTr("Breakpoints")},
                        {icon: "🚫", label: qsTr("Blacklist")},
                        {icon: "📂", label: qsTr("Map Local")},
                        {icon: "🔀", label: qsTr("Map Remote")},
                        {icon: "🐌", label: qsTr("Throttle")}
                    ]
                    property int currentIndex: 0
                    delegate: Rectangle {
                        width: parent.width; height: 36; radius: 4
                        color: rulesNavRepeater.currentIndex === index ? Qt.alpha(root._accentColor, 0.2) : rnMa.containsMouse ? root._headerHover : "transparent"
                        Row {
                            anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                            Text { text: modelData.icon; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter }
                            UTText { text: modelData.label; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.bold: rulesNavRepeater.currentIndex === index; anchors.verticalCenter: parent.verticalCenter }
                        }
                        MouseArea { id: rnMa; anchors.fill: parent; hoverEnabled: true; onClicked: rulesNavRepeater.currentIndex = index }
                    }
                }
            }
        }

        // ── Right content ──
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4

            StackLayout {
                anchors.fill: parent; anchors.margins: 12
                currentIndex: rulesNavRepeater.currentIndex

                // ─── Mock Response ───
                ColumnLayout {
                    spacing: 10
                    UTText { text: qsTr("Mock Response Rules"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                    UTText { text: qsTr("Match requests by URL pattern and return a custom response. Mocked requests will appear in Capture with a \"MOCK\" tag."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    GridLayout {
                        columns: 4; columnSpacing: 8; rowSpacing: 6; Layout.fillWidth: true
                        UTText { text: qsTr("URL Pattern:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                        UTTextField { id: mockUrl; Layout.fillWidth: true; Layout.columnSpan: 2; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("e.g. /api/user.*") }
                        UTButton { text: qsTr("🧪 Test"); onClicked: root.testPattern(mockUrl.text) }
                        UTText { text: qsTr("Status:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                        UTTextField { id: mockStatus; implicitWidth: 70; fontEnum: UIFontToken.Monospace_Text; placeholderText: "200"; validator: IntValidator { bottom: 100; top: 599 } }
                        UTText { text: qsTr("Content-Type:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                        UTTextField { id: mockCt; implicitWidth: 180; fontEnum: UIFontToken.Monospace_Text; placeholderText: "application/json"; text: "application/json" }
                        UTText { text: qsTr("Headers:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                        UTTextField { id: mockHeaders; Layout.fillWidth: true; Layout.columnSpan: 3; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("X-Custom: value  (one per line or comma-separated)") }
                        UTText { text: qsTr("Body:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                        UTTextField { id: mockBody; Layout.fillWidth: true; Layout.columnSpan: 3; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("{\"message\": \"mocked\"}") }
                    }
                    RowLayout {
                        spacing: 8
                        UTButton { text: qsTr("+ Add Rule"); onClicked: { root.controller.rulesManager.addMockRule(mockUrl.text, parseInt(mockStatus.text)||200, mockCt.text||"application/json", mockBody.text, mockHeaders.text); mockUrl.text=""; mockStatus.text=""; mockBody.text=""; mockHeaders.text=""; mockListView.model = root.controller.rulesManager.getMockRules() } }
                        Item { Layout.fillWidth: true }
                        UTButton { text: qsTr("Clear All"); onClicked: { root.controller.rulesManager.clearMockRules(); mockListView.model = root.controller.rulesManager.getMockRules() } }
                    }
                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        color: root._inputBg; border.color: root._inputBorder; border.width: 1; radius: 4
                        ListView {
                            id: mockListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                            model: root.controller.rulesManager.getMockRules()
                            delegate: Rectangle {
                                width: mockListView.width; height: 32; radius: 3
                                color: mockDelegateMa.containsMouse ? Qt.alpha(root._accentColor, 0.12) : (index % 2 === 0 ? "transparent" : Qt.darker(root._sectionBg, 1.03))
                                MouseArea {
                                    id: mockDelegateMa; anchors.fill: parent; hoverEnabled: true
                                    onDoubleClicked: root.showMockDetail(modelData)
                                }
                                RowLayout {
                                    anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                    Rectangle { width: 40; height: 18; radius: 3; color: root._accentColor; UTText { anchors.centerIn: parent; text: modelData.status_code || "200"; fontEnum: UIFontToken.Caption_Text; color: "white" } }
                                    UTText { Layout.fillWidth: true; text: modelData.url_pattern || ""; fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight }
                                    UTText { visible: (modelData.headers || "").length > 0; text: "📋"; font.pixelSize: 12; ToolTip.text: qsTr("Has custom headers"); ToolTip.visible: false }
                                    UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { root.controller.rulesManager.removeMockRule(index); mockListView.model = root.controller.rulesManager.getMockRules() } }
                                }
                            }
                            UTText { anchors.centerIn: parent; visible: parent.count === 0; text: qsTr("No mock rules. Add one above.\nDouble-click a rule to view details."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.5; horizontalAlignment: Text.AlignHCenter }
                        }
                    }
                }

                // ─── Breakpoints ───
                ColumnLayout {
                    spacing: 10
                    UTText { text: qsTr("Breakpoint Rules"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                    UTText { text: qsTr("Pause matching requests so you can inspect or modify them before forwarding."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; wrapMode: Text.WordWrap; Layout.fillWidth: true }

                    Rectangle {
                        Layout.fillWidth: true; height: 32; radius: 4; visible: !root.controller.interceptEnabled
                        color: Qt.alpha("#FF9800", 0.1); border.color: Qt.alpha("#FF9800", 0.3); border.width: 1
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 6
                            UTText { text: qsTr("⚠️ Interception is OFF — breakpoints won't take effect."); fontEnum: UIFontToken.Caption_Text; color: "#FF9800"; Layout.fillWidth: true }
                            UTButton { text: qsTr("Enable"); implicitHeight: 24; onClicked: root.controller.interceptEnabled = true }
                        }
                    }

                    RowLayout {
                        spacing: 8; Layout.fillWidth: true
                        UTTextField { id: bpUrl; Layout.fillWidth: true; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("URL pattern (regex)") }
                        ComboBox { id: bpMethod; model: ["ANY","GET","POST","PUT","DELETE"]; implicitWidth: 100 }
                        UTButton { text: qsTr("+ Add"); onClicked: { root.controller.rulesManager.addBreakpointRule(bpUrl.text, bpMethod.currentText); bpUrl.text=""; bpListView.model = root.controller.rulesManager.getBreakpointRules(); if (!root.controller.interceptEnabled) root.controller.interceptEnabled = true } }
                        UTButton { text: qsTr("🧪 Test"); onClicked: root.testPattern(bpUrl.text) }
                        Item { Layout.fillWidth: true }
                        UTButton { text: qsTr("Clear All"); onClicked: { root.controller.rulesManager.clearBreakpointRules(); bpListView.model = root.controller.rulesManager.getBreakpointRules() } }
                    }
                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 60
                        color: root._inputBg; border.color: root._inputBorder; border.width: 1; radius: 4
                        ListView {
                            id: bpListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                            model: root.controller.rulesManager.getBreakpointRules()
                            delegate: Rectangle {
                                width: bpListView.width; height: 28; radius: 3; color: index % 2 === 0 ? "transparent" : Qt.darker(root._sectionBg, 1.03)
                                RowLayout {
                                    anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                    Rectangle { width: 44; height: 18; radius: 3; color: "#9C27B0"; UTText { anchors.centerIn: parent; text: modelData.method || "ANY"; fontEnum: UIFontToken.Caption_Text; color: "white" } }
                                    UTText { Layout.fillWidth: true; text: modelData.url_pattern || ""; fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight }
                                    UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { root.controller.rulesManager.removeBreakpointRule(index); bpListView.model = root.controller.rulesManager.getBreakpointRules() } }
                                }
                            }
                            UTText { anchors.centerIn: parent; visible: parent.count === 0; text: qsTr("No breakpoint rules."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.5 }
                        }
                    }

                    // ─── Paused Requests Panel ───
                    UTText {
                        text: qsTr("Paused Requests (%1)").arg(root.interceptedFlowsModel.count)
                        fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title
                        visible: root.interceptedFlowsModel.count > 0
                    }
                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: Math.min(root.interceptedFlowsModel.count * 36 + 8, 180)
                        visible: root.interceptedFlowsModel.count > 0
                        color: Qt.alpha("#F44336", 0.05); border.color: Qt.alpha("#F44336", 0.3); border.width: 1; radius: 4
                        ListView {
                            id: pausedListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                            model: root.interceptedFlowsModel
                            delegate: Rectangle {
                                width: pausedListView.width; height: 32; radius: 3
                                color: pausedMa.containsMouse ? Qt.alpha("#F44336", 0.1) : "transparent"
                                MouseArea { id: pausedMa; anchors.fill: parent; hoverEnabled: true }
                                RowLayout {
                                    anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 6
                                    Rectangle { width: 8; height: 8; radius: 4; color: "#F44336"; anchors.verticalCenter: parent.verticalCenter }
                                    Rectangle {
                                        width: 44; height: 18; radius: 3; anchors.verticalCenter: parent.verticalCenter
                                        color: { switch(model.method) { case "GET": return "#4CAF50"; case "POST": return "#2196F3"; case "PUT": return "#FF9800"; case "DELETE": return "#F44336"; default: return "#757575" } }
                                        UTText { anchors.centerIn: parent; text: model.method; fontEnum: UIFontToken.Caption_Text; color: "white"; font.bold: true }
                                    }
                                    UTText { Layout.fillWidth: true; text: model.url; fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideMiddle }
                                    UTButton {
                                        text: qsTr("▶ Forward"); implicitHeight: 24
                                        onClicked: { root.controller.resumeRequest(model.flowId); root.interceptedFlowsModel.remove(index) }
                                    }
                                    UTButton {
                                        text: qsTr("✕ Drop"); implicitHeight: 24
                                        onClicked: { root.controller.dropRequest(model.flowId); root.interceptedFlowsModel.remove(index) }
                                    }
                                }
                            }
                        }
                    }
                    RowLayout {
                        visible: root.interceptedFlowsModel.count > 1; spacing: 8
                        UTButton {
                            text: qsTr("▶ Forward All (%1)").arg(root.interceptedFlowsModel.count)
                            onClicked: {
                                for (var i = root.interceptedFlowsModel.count - 1; i >= 0; i--)
                                    root.controller.resumeRequest(root.interceptedFlowsModel.get(i).flowId)
                                root.interceptedFlowsModel.clear()
                            }
                        }
                        UTButton {
                            text: qsTr("✕ Drop All (%1)").arg(root.interceptedFlowsModel.count)
                            onClicked: {
                                for (var i = root.interceptedFlowsModel.count - 1; i >= 0; i--)
                                    root.controller.dropRequest(root.interceptedFlowsModel.get(i).flowId)
                                root.interceptedFlowsModel.clear()
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true; implicitHeight: bpHelpCol.implicitHeight + 16; radius: 4
                        color: Qt.alpha(root._accentColor, 0.05); border.color: Qt.alpha(root._accentColor, 0.2); border.width: 1
                        ColumnLayout {
                            id: bpHelpCol
                            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 8; spacing: 4
                            UTText { text: qsTr("How to use Breakpoints:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                            UTText { text: qsTr("1. Add a URL pattern rule above (regex)"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text }
                            UTText { text: qsTr("2. Interception is auto-enabled when you add a rule"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text }
                            UTText { text: qsTr("3. When a matching request arrives, it pauses and appears in \"Paused Requests\""); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text }
                            UTText { text: qsTr("4. Click \"▶ Forward\" to let it through, or \"✕ Drop\" to block it (returns 502)"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text }
                            UTText { text: qsTr("5. Paused requests auto-forward after 5 minutes timeout"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text }
                        }
                    }
                }

                // ─── Blacklist ───
                ColumnLayout {
                    spacing: 10
                    UTText { text: qsTr("Blacklist Rules"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                    UTText { text: qsTr("Block matching requests entirely. They will receive a 403 response."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    RowLayout {
                        spacing: 8; Layout.fillWidth: true
                        UTTextField { id: blUrl; Layout.fillWidth: true; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("URL pattern to block (regex)") }
                        UTButton { text: qsTr("+ Add"); onClicked: { root.controller.rulesManager.addBlacklistRule(blUrl.text); blUrl.text=""; blListView.model = root.controller.rulesManager.getBlacklistRules() } }
                    }
                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        color: root._inputBg; border.color: root._inputBorder; border.width: 1; radius: 4
                        ListView {
                            id: blListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                            model: root.controller.rulesManager.getBlacklistRules()
                            delegate: Rectangle {
                                width: blListView.width; height: 28; radius: 3; color: index % 2 === 0 ? "transparent" : Qt.darker(root._sectionBg, 1.03)
                                RowLayout {
                                    anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                    Rectangle { width: 14; height: 14; radius: 7; color: "#F44336"; anchors.verticalCenter: parent.verticalCenter }
                                    UTText { Layout.fillWidth: true; text: modelData.url_pattern || modelData; fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight }
                                    UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { root.controller.rulesManager.removeBlacklistRule(index); blListView.model = root.controller.rulesManager.getBlacklistRules() } }
                                }
                            }
                            UTText { anchors.centerIn: parent; visible: parent.count === 0; text: qsTr("No blacklist rules."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.5 }
                        }
                    }
                }

                // ─── Map Local ───
                ColumnLayout {
                    spacing: 10
                    UTText { text: qsTr("Map Local Rules"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                    UTText { text: qsTr("Serve a local file instead of the remote response for matching URLs."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    RowLayout {
                        spacing: 8; Layout.fillWidth: true
                        UTTextField { id: mlUrl; Layout.fillWidth: true; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("URL pattern (regex)") }
                        UTTextField { id: mlPath; Layout.fillWidth: true; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("Local file path") }
                        UTButton { text: qsTr("+ Add"); onClicked: { root.controller.rulesManager.addMapLocalRule(mlUrl.text, mlPath.text); mlUrl.text=""; mlPath.text=""; mlListView.model = root.controller.rulesManager.getMapLocalRules() } }
                    }
                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        color: root._inputBg; border.color: root._inputBorder; border.width: 1; radius: 4
                        ListView {
                            id: mlListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                            model: root.controller.rulesManager.getMapLocalRules()
                            delegate: Rectangle {
                                width: mlListView.width; height: 28; radius: 3; color: index % 2 === 0 ? "transparent" : Qt.darker(root._sectionBg, 1.03)
                                RowLayout {
                                    anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                    UTText { Layout.fillWidth: true; text: (modelData.url_pattern||"") + " → " + (modelData.local_path||""); fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight }
                                    UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { root.controller.rulesManager.removeMapLocalRule(index); mlListView.model = root.controller.rulesManager.getMapLocalRules() } }
                                }
                            }
                            UTText { anchors.centerIn: parent; visible: parent.count === 0; text: qsTr("No map local rules."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.5 }
                        }
                    }
                }

                // ─── Map Remote ───
                ColumnLayout {
                    spacing: 10
                    UTText { text: qsTr("Map Remote Rules"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                    UTText { text: qsTr("Redirect matching requests to a different URL."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    RowLayout {
                        spacing: 8; Layout.fillWidth: true
                        UTTextField { id: mrSrc; Layout.fillWidth: true; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("Source URL pattern (regex)") }
                        UTTextField { id: mrDest; Layout.fillWidth: true; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("Destination URL") }
                        UTButton { text: qsTr("+ Add"); onClicked: { root.controller.rulesManager.addMapRemoteRule(mrSrc.text, mrDest.text); mrSrc.text=""; mrDest.text=""; mrListView.model = root.controller.rulesManager.getMapRemoteRules() } }
                    }
                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        color: root._inputBg; border.color: root._inputBorder; border.width: 1; radius: 4
                        ListView {
                            id: mrListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                            model: root.controller.rulesManager.getMapRemoteRules()
                            delegate: Rectangle {
                                width: mrListView.width; height: 28; radius: 3; color: index % 2 === 0 ? "transparent" : Qt.darker(root._sectionBg, 1.03)
                                RowLayout {
                                    anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                    UTText { Layout.fillWidth: true; text: (modelData.src_pattern||"") + " → " + (modelData.dest_url||""); fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight }
                                    UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { root.controller.rulesManager.removeMapRemoteRule(index); mrListView.model = root.controller.rulesManager.getMapRemoteRules() } }
                                }
                            }
                            UTText { anchors.centerIn: parent; visible: parent.count === 0; text: qsTr("No map remote rules."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.5 }
                        }
                    }
                }

                // ─── Throttle ───
                ColumnLayout {
                    spacing: 10
                    UTText { text: qsTr("Bandwidth Throttle"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                    UTText { text: qsTr("Simulate slow network conditions by limiting bandwidth."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; wrapMode: Text.WordWrap; Layout.fillWidth: true }

                    UTCheckBox { id: throttleEnabled; text: qsTr("Enable Throttle") }

                    GridLayout {
                        columns: 3; columnSpacing: 12; rowSpacing: 8; Layout.fillWidth: true
                        enabled: throttleEnabled.checked; opacity: throttleEnabled.checked ? 1.0 : 0.5

                        UTText { text: qsTr("Download:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                        Slider { id: dlSlider; from: 0; to: 10000; stepSize: 50; value: 0; Layout.fillWidth: true }
                        UTText { text: dlSlider.value > 0 ? qsTr("%1 KB/s").arg(dlSlider.value) : qsTr("Unlimited"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; Layout.preferredWidth: 100 }

                        UTText { text: qsTr("Upload:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                        Slider { id: ulSlider; from: 0; to: 10000; stepSize: 50; value: 0; Layout.fillWidth: true }
                        UTText { text: ulSlider.value > 0 ? qsTr("%1 KB/s").arg(ulSlider.value) : qsTr("Unlimited"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; Layout.preferredWidth: 100 }
                    }

                    RowLayout {
                        spacing: 12
                        UTText { text: qsTr("Presets:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                        Repeater {
                            model: [
                                {label: "3G", dl: 750, ul: 250},
                                {label: "4G", dl: 4000, ul: 3000},
                                {label: "WiFi", dl: 0, ul: 0}
                            ]
                            delegate: UTButton {
                                text: modelData.label
                                onClicked: { throttleEnabled.checked = modelData.dl > 0; dlSlider.value = modelData.dl; ulSlider.value = modelData.ul }
                            }
                        }
                        Item { Layout.fillWidth: true }
                        UTButton {
                            text: qsTr("Apply")
                            onClicked: root.controller.rulesManager.setThrottle(throttleEnabled.checked, dlSlider.value, ulSlider.value)
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
}
