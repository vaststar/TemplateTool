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

    // ── Theme helpers ──
    readonly property color _inputBg:    UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background, UIColorState.Normal)
    readonly property color _inputText:  UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text, UIColorState.Normal)
    readonly property color _inputBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Normal)
    readonly property color _inputPlaceholder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder, UIColorState.Normal)
    readonly property color _sectionBg:  UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border, UIColorState.Normal)
    readonly property color _sectionTitle: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Title, UIColorState.Normal)
    readonly property font  _inputFont:  UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    readonly property font  _monoFont:   Qt.font({family: "Consolas", pixelSize: _inputFont.pixelSize})
    readonly property color _headerBg:   Qt.darker(_sectionBg, 1.08)
    readonly property color _headerHover: Qt.darker(_sectionBg, 1.15)
    readonly property color _accentColor: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)

    // ── Draggable column widths ──
    property real colMethodW: 64
    property real colStatusW: 56
    property real colTimeW:   65
    property real colSizeW:   72
    property real colProcessW: 90

    // ── Method multi-select filter ──
    property var selectedMethods: []   // empty = ALL
    function methodFilterString() {
        return selectedMethods.length === 0 ? "ALL" : selectedMethods.join(",")
    }
    function toggleMethod(m) {
        var arr = selectedMethods.slice()
        var idx = arr.indexOf(m)
        if (idx >= 0) arr.splice(idx, 1); else arr.push(m)
        selectedMethods = arr
    }
    function isMethodSelected(m) {
        return selectedMethods.length === 0 || selectedMethods.indexOf(m) >= 0
    }

    // ── Status multi-select filter ──
    property var selectedStatuses: []   // empty = ALL
    function statusFilterString() {
        return selectedStatuses.length === 0 ? "ALL" : selectedStatuses.join(",")
    }
    function toggleStatus(s) {
        var arr = selectedStatuses.slice()
        var idx = arr.indexOf(s)
        if (idx >= 0) arr.splice(idx, 1); else arr.push(s)
        selectedStatuses = arr
    }

    // ── Process multi-select filter ──
    property var selectedProcesses: []   // empty = ALL
    function processFilterString() {
        return selectedProcesses.length === 0 ? "ALL" : selectedProcesses.join(",")
    }
    function toggleProcess(p) {
        var arr = selectedProcesses.slice()
        var idx = arr.indexOf(p)
        if (idx >= 0) arr.splice(idx, 1); else arr.push(p)
        selectedProcesses = arr
    }

    // ── Helper: themed input background ──
    component ThemedInput : TextField {
        font: proxyPanel._monoFont
        color: proxyPanel._inputText
        placeholderTextColor: proxyPanel._inputPlaceholder
        background: Rectangle {
            color: proxyPanel._inputBg
            border.color: parent.activeFocus ? proxyPanel._accentColor : proxyPanel._inputBorder
            border.width: 1; radius: 4
        }
    }

    // ── Helper: visible checkbox ──
    component ThemedCheckBox : CheckBox {
        font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
        indicator: Rectangle {
            implicitWidth: 18; implicitHeight: 18
            x: 2; y: parent.height / 2 - height / 2; radius: 3
            color: parent.checked ? proxyPanel._accentColor : proxyPanel._inputBg
            border.color: parent.checked ? proxyPanel._accentColor : proxyPanel._inputBorder
            border.width: 1
            Text {
                anchors.centerIn: parent; text: "✓"
                font.pixelSize: 13; font.bold: true
                color: "white"; visible: parent.parent.checked
            }
        }
        contentItem: UTText {
            text: parent.text; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text
            leftPadding: parent.indicator.width + 6
            verticalAlignment: Text.AlignVCenter
        }
    }

    // ── Helper: column drag handle ──
    component ColDragHandle : Rectangle {
        property string targetCol  // "method", "status", "time", "size", "process"
        width: 5
        height: parent.height
        color: handleMa.containsMouse ? proxyPanel._accentColor : "transparent"
        radius: 1
        MouseArea {
            id: handleMa
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.SplitHCursor
            property real startX: 0
            property real startW: 0
            onPressed: function(mouse) {
                startX = mapToGlobal(mouse.x, 0).x
                switch (targetCol) {
                case "method":  startW = proxyPanel.colMethodW;  break
                case "status":  startW = proxyPanel.colStatusW;  break
                case "time":    startW = proxyPanel.colTimeW;    break
                case "size":    startW = proxyPanel.colSizeW;    break
                case "process": startW = proxyPanel.colProcessW; break
                }
            }
            onPositionChanged: function(mouse) {
                if (!pressed) return
                var dx = mapToGlobal(mouse.x, 0).x - startX
                // Handles for method/status sit on the RIGHT edge → +dx widens
                // Handles for time/size/process sit on the LEFT edge → −dx widens
                var leftEdge = (targetCol === "time" || targetCol === "size" || targetCol === "process")
                var nw = Math.max(36, startW + (leftEdge ? -dx : dx))
                switch (targetCol) {
                case "method":  proxyPanel.colMethodW  = nw; break
                case "status":  proxyPanel.colStatusW  = nw; break
                case "time":    proxyPanel.colTimeW    = nw; break
                case "size":    proxyPanel.colSizeW    = nw; break
                case "process": proxyPanel.colProcessW = nw; break
                }
            }
        }
    }

    ProxyFilterModel {
        id: filterModel
        sourceModel: controller.requestModel
        filterUrl: filterInput.text
        filterMethod: proxyPanel.methodFilterString()
        filterStatus: proxyPanel.statusFilterString()
        filterProcess: proxyPanel.processFilterString()
    }

    // ── Intercepted (paused) flows model ──
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
        // Control Bar (shared across all tabs)
        // ════════════════════════════════════════════
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            UTButton {
                text: controller.proxyRunning ? qsTr("■ Stop") : qsTr("▶ Start")
                onClicked: controller.proxyRunning ? controller.stopProxy() : controller.startProxy()
            }

            UTText { text: qsTr("Port:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
            TextField {
                id: proxyPortInput
                implicitWidth: 60
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
        // Top-level TabBar: Capture | Rules | Settings
        // ════════════════════════════════════════════
        TabBar {
            id: mainTabBar
            Layout.fillWidth: true
            TabButton { text: qsTr("📡 Capture") }
            TabButton { text: qsTr("📋 Rules") }
            TabButton { text: qsTr("⚙ Settings") }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: mainTabBar.currentIndex

            // ╔══════════════════════════════════════╗
            // ║       TAB 0 — CAPTURE                ║
            // ╚══════════════════════════════════════╝
            Item {
                // Paused requests notification banner
                Rectangle {
                    id: pausedBanner
                    visible: interceptedFlowsModel.count > 0
                    z: 10; anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right
                    height: visible ? 32 : 0; radius: 4
                    color: Qt.alpha("#F44336", 0.12); border.color: Qt.alpha("#F44336", 0.4); border.width: 1
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 6
                        Text { text: "🔴"; font.pixelSize: 12 }
                        UTText {
                            text: qsTr("%1 request(s) paused by breakpoint").arg(interceptedFlowsModel.count)
                            fontEnum: UIFontToken.Caption_Text; color: "#F44336"; font.bold: true; Layout.fillWidth: true
                        }
                        UTButton {
                            text: qsTr("Go to Breakpoints"); implicitHeight: 24
                            onClicked: { mainTabBar.currentIndex = 1; rulesNavRepeater.currentIndex = 1 }
                        }
                        UTButton {
                            text: qsTr("Forward All"); implicitHeight: 24
                            onClicked: {
                                for (var i = interceptedFlowsModel.count - 1; i >= 0; i--)
                                    controller.resumeRequest(interceptedFlowsModel.get(i).flowId)
                                interceptedFlowsModel.clear()
                            }
                        }
                    }
                }

                SplitView {
                    anchors.fill: parent; anchors.topMargin: pausedBanner.visible ? 36 : 0
                    orientation: Qt.Horizontal

                    // ────── Left: Capture List ──────
                    Rectangle {
                        SplitView.preferredWidth: proxyPanel.width * 0.48
                        SplitView.minimumWidth: 320
                        color: proxyPanel._sectionBg
                        border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4

                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 1; spacing: 0

                            // ─── Column Headers with drag handles ───
                            Rectangle {
                                Layout.fillWidth: true; height: 30
                                color: proxyPanel._headerBg; radius: 3

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8; anchors.rightMargin: 8

                                    // METHOD header
                                    Rectangle {
                                        width: proxyPanel.colMethodW; height: parent.height
                                        color: methodHeaderMa.containsMouse ? proxyPanel._headerHover : "transparent"; radius: 2
                                        Row {
                                            anchors.centerIn: parent; spacing: 2
                                            UTText { text: qsTr("Method"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                            UTText {
                                                text: proxyPanel.selectedMethods.length > 0 ? "▾" : "▿"; fontEnum: UIFontToken.Caption_Text
                                                color: proxyPanel.selectedMethods.length > 0 ? proxyPanel._accentColor : proxyPanel._sectionTitle
                                            }
                                        }
                                        MouseArea { id: methodHeaderMa; anchors.fill: parent; hoverEnabled: true; onClicked: methodPopup.open() }
                                        Popup {
                                            id: methodPopup; y: parent.height + 2; width: 160; padding: 4
                                            background: Rectangle { color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4 }
                                            Column {
                                                width: parent.width; spacing: 1
                                                // ALL option (clears selection)
                                                Rectangle {
                                                    width: parent.width; height: 26
                                                    color: allMa.containsMouse ? proxyPanel._headerHover : "transparent"; radius: 3
                                                    Row {
                                                        anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                                        UTText { text: qsTr("ALL"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.bold: proxyPanel.selectedMethods.length === 0; anchors.verticalCenter: parent.verticalCenter }
                                                    }
                                                    UTText { anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; visible: proxyPanel.selectedMethods.length === 0; text: "✓"; color: proxyPanel._accentColor; fontEnum: UIFontToken.Body_Text }
                                                    MouseArea { id: allMa; anchors.fill: parent; hoverEnabled: true; onClicked: proxyPanel.selectedMethods = [] }
                                                }
                                                // Individual methods with checkboxes
                                                Repeater {
                                                    model: ["GET","POST","PUT","DELETE","PATCH","OPTIONS","HEAD"]
                                                    delegate: Rectangle {
                                                        width: parent.width; height: 26
                                                        color: mhMa.containsMouse ? proxyPanel._headerHover : "transparent"; radius: 3
                                                        Row {
                                                            anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                                            Rectangle {
                                                                width: 14; height: 14; radius: 2; anchors.verticalCenter: parent.verticalCenter
                                                                color: proxyPanel.selectedMethods.indexOf(modelData) >= 0 ? proxyPanel._accentColor : "transparent"
                                                                border.color: proxyPanel.selectedMethods.indexOf(modelData) >= 0 ? proxyPanel._accentColor : proxyPanel._inputBorder; border.width: 1
                                                                Text { anchors.centerIn: parent; text: "✓"; font.pixelSize: 10; color: "white"; visible: proxyPanel.selectedMethods.indexOf(modelData) >= 0 }
                                                            }
                                                            Rectangle {
                                                                width: 6; height: 6; radius: 3; anchors.verticalCenter: parent.verticalCenter
                                                                color: { switch(modelData) { case "GET": return "#4CAF50"; case "POST": return "#2196F3"; case "PUT": return "#FF9800"; case "DELETE": return "#F44336"; case "PATCH": return "#9C27B0"; default: return "#757575" } }
                                                            }
                                                            UTText { text: modelData; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; anchors.verticalCenter: parent.verticalCenter }
                                                        }
                                                        MouseArea { id: mhMa; anchors.fill: parent; hoverEnabled: true; onClicked: proxyPanel.toggleMethod(modelData) }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    ColDragHandle { targetCol: "method" }

                                    // STATUS header (multi-select)
                                    Rectangle {
                                        width: proxyPanel.colStatusW; height: parent.height
                                        color: statusHeaderMa.containsMouse ? proxyPanel._headerHover : "transparent"; radius: 2
                                        Row {
                                            anchors.centerIn: parent; spacing: 2
                                            UTText { text: qsTr("Status"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                            UTText {
                                                text: proxyPanel.selectedStatuses.length > 0 ? "▾" : "▿"; fontEnum: UIFontToken.Caption_Text
                                                color: proxyPanel.selectedStatuses.length > 0 ? proxyPanel._accentColor : proxyPanel._sectionTitle
                                            }
                                        }
                                        MouseArea { id: statusHeaderMa; anchors.fill: parent; hoverEnabled: true; onClicked: statusPopup.open() }
                                        Popup {
                                            id: statusPopup; y: parent.height + 2; width: 130; padding: 4
                                            background: Rectangle { color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4 }
                                            Column {
                                                width: parent.width; spacing: 1
                                                // ALL option (clears selection)
                                                Rectangle {
                                                    width: parent.width; height: 26
                                                    color: allStatusMa.containsMouse ? proxyPanel._headerHover : "transparent"; radius: 3
                                                    Row {
                                                        anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                                        UTText { text: qsTr("ALL"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.bold: proxyPanel.selectedStatuses.length === 0; anchors.verticalCenter: parent.verticalCenter }
                                                    }
                                                    UTText { anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; visible: proxyPanel.selectedStatuses.length === 0; text: "✓"; color: proxyPanel._accentColor; fontEnum: UIFontToken.Body_Text }
                                                    MouseArea { id: allStatusMa; anchors.fill: parent; hoverEnabled: true; onClicked: proxyPanel.selectedStatuses = [] }
                                                }
                                                // Individual status ranges with checkboxes
                                                Repeater {
                                                    model: ["2xx","3xx","4xx","5xx"]
                                                    delegate: Rectangle {
                                                        width: parent.width; height: 26
                                                        color: shMa.containsMouse ? proxyPanel._headerHover : "transparent"; radius: 3
                                                        Row {
                                                            anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                                            Rectangle {
                                                                width: 14; height: 14; radius: 2; anchors.verticalCenter: parent.verticalCenter
                                                                color: proxyPanel.selectedStatuses.indexOf(modelData) >= 0 ? proxyPanel._accentColor : "transparent"
                                                                border.color: proxyPanel.selectedStatuses.indexOf(modelData) >= 0 ? proxyPanel._accentColor : proxyPanel._inputBorder; border.width: 1
                                                                Text { anchors.centerIn: parent; text: "✓"; font.pixelSize: 10; color: "white"; visible: proxyPanel.selectedStatuses.indexOf(modelData) >= 0 }
                                                            }
                                                            Rectangle {
                                                                width: 6; height: 6; radius: 3; anchors.verticalCenter: parent.verticalCenter
                                                                color: { switch(modelData) { case "2xx": return "#4CAF50"; case "3xx": return "#FF9800"; case "4xx": return "#F44336"; case "5xx": return "#D32F2F"; default: return "#757575" } }
                                                            }
                                                            UTText { text: modelData; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; anchors.verticalCenter: parent.verticalCenter }
                                                        }
                                                        MouseArea { id: shMa; anchors.fill: parent; hoverEnabled: true; onClicked: proxyPanel.toggleStatus(modelData) }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    ColDragHandle { targetCol: "status" }

                                    // URL header + inline filter
                                    Item {
                                        width: parent.width - proxyPanel.colMethodW - proxyPanel.colStatusW - proxyPanel.colTimeW - proxyPanel.colSizeW - proxyPanel.colProcessW - 5*5 // subtract handles
                                        height: parent.height
                                        Row {
                                            anchors.fill: parent; spacing: 4; anchors.verticalCenter: parent.verticalCenter
                                            UTText { text: qsTr("URL"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title; anchors.verticalCenter: parent.verticalCenter }
                                            TextField {
                                                id: filterInput
                                                width: parent.width - 40; height: 22
                                                anchors.verticalCenter: parent.verticalCenter
                                                placeholderText: qsTr("Filter..."); placeholderTextColor: proxyPanel._inputPlaceholder
                                                font: Qt.font({family: "Consolas", pixelSize: proxyPanel._inputFont.pixelSize - 2})
                                                color: proxyPanel._inputText
                                                leftPadding: 4; rightPadding: 4; topPadding: 2; bottomPadding: 2
                                                background: Rectangle { color: proxyPanel._inputBg; border.color: filterInput.activeFocus ? proxyPanel._accentColor : proxyPanel._inputBorder; border.width: 1; radius: 3 }
                                            }
                                        }
                                    }
                                    ColDragHandle { targetCol: "time" }

                                    // TIME header
                                    Item {
                                        width: proxyPanel.colTimeW; height: parent.height
                                        UTText { anchors.centerIn: parent; text: qsTr("Time"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                    }
                                    ColDragHandle { targetCol: "size" }

                                    // SIZE header
                                    Item {
                                        width: proxyPanel.colSizeW; height: parent.height
                                        UTText { anchors.centerIn: parent; text: qsTr("Size"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                    }
                                    ColDragHandle { targetCol: "process" }

                                    // PROCESS header (multi-select)
                                    Rectangle {
                                        width: proxyPanel.colProcessW; height: parent.height
                                        color: processHeaderMa.containsMouse ? proxyPanel._headerHover : "transparent"; radius: 2
                                        Row {
                                            anchors.centerIn: parent; spacing: 2
                                            UTText { text: qsTr("Process"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                            UTText {
                                                text: proxyPanel.selectedProcesses.length > 0 ? "▾" : "▿"; fontEnum: UIFontToken.Caption_Text
                                                color: proxyPanel.selectedProcesses.length > 0 ? proxyPanel._accentColor : proxyPanel._sectionTitle
                                            }
                                        }
                                        MouseArea { id: processHeaderMa; anchors.fill: parent; hoverEnabled: true; onClicked: { processListModel.refresh(); processPopup.open() } }

                                        // Dynamic model that refreshes from the source model each time popup opens
                                        ListModel { id: processListModel
                                            function refresh() {
                                                clear()
                                                var names = controller.requestModel.uniqueProcessNames()
                                                for (var i = 0; i < names.length; i++)
                                                    append({"name": names[i]})
                                            }
                                        }

                                        Popup {
                                            id: processPopup; y: parent.height + 2; width: 200; padding: 4
                                            background: Rectangle { color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4 }
                                            Column {
                                                width: parent.width; spacing: 1
                                                // ALL option (clears selection)
                                                Rectangle {
                                                    width: parent.width; height: 26
                                                    color: allProcMa.containsMouse ? proxyPanel._headerHover : "transparent"; radius: 3
                                                    Row {
                                                        anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                                        UTText { text: qsTr("ALL"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.bold: proxyPanel.selectedProcesses.length === 0; anchors.verticalCenter: parent.verticalCenter }
                                                    }
                                                    UTText { anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; visible: proxyPanel.selectedProcesses.length === 0; text: "✓"; color: proxyPanel._accentColor; fontEnum: UIFontToken.Body_Text }
                                                    MouseArea { id: allProcMa; anchors.fill: parent; hoverEnabled: true; onClicked: proxyPanel.selectedProcesses = [] }
                                                }
                                                // Scrollable list of unique process names
                                                ListView {
                                                    width: parent.width; height: Math.min(processListModel.count * 26, 200)
                                                    clip: true; model: processListModel
                                                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                                                    delegate: Rectangle {
                                                        width: parent ? parent.width : 0; height: 26
                                                        color: phMa.containsMouse ? proxyPanel._headerHover : "transparent"; radius: 3
                                                        Row {
                                                            anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                                            Rectangle {
                                                                width: 14; height: 14; radius: 2; anchors.verticalCenter: parent.verticalCenter
                                                                color: proxyPanel.selectedProcesses.indexOf(model.name) >= 0 ? proxyPanel._accentColor : "transparent"
                                                                border.color: proxyPanel.selectedProcesses.indexOf(model.name) >= 0 ? proxyPanel._accentColor : proxyPanel._inputBorder; border.width: 1
                                                                Text { anchors.centerIn: parent; text: "✓"; font.pixelSize: 10; color: "white"; visible: proxyPanel.selectedProcesses.indexOf(model.name) >= 0 }
                                                            }
                                                            UTText { text: model.name; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; anchors.verticalCenter: parent.verticalCenter; elide: Text.ElideRight; width: parent.width - 32 }
                                                        }
                                                        MouseArea { id: phMa; anchors.fill: parent; hoverEnabled: true; onClicked: proxyPanel.toggleProcess(model.name) }
                                                    }
                                                }
                                                // Empty state
                                                UTText {
                                                    visible: processListModel.count === 0
                                                    width: parent.width; height: 26
                                                    text: qsTr("No processes captured"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
                                                    opacity: 0.5; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // ─── ListView ───
                            ListView {
                                id: requestListView
                                Layout.fillWidth: true; Layout.fillHeight: true
                                clip: true; model: filterModel
                                currentIndex: controller.selectedIndex
                                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                                delegate: Rectangle {
                                    width: requestListView.width; height: 26
                                    color: index === requestListView.currentIndex
                                        ? Qt.alpha(proxyPanel._accentColor, 0.2)
                                        : (index % 2 === 0 ? "transparent" : Qt.darker(proxyPanel._sectionBg, 1.03))

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: { requestListView.currentIndex = index; controller.selectedIndex = filterModel.mapToSource(filterModel.index(index, 0)).row }
                                    }

                                    Row {
                                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 5

                                        // Intercept badge
                                        Rectangle {
                                            visible: model.isIntercepted === true
                                            width: 18; height: 18; radius: 9; anchors.verticalCenter: parent.verticalCenter
                                            color: "#F44336"
                                            Text { anchors.centerIn: parent; text: "⏸"; font.pixelSize: 10; color: "white" }
                                        }

                                        // Method badge
                                        Rectangle {
                                            width: proxyPanel.colMethodW - 5; height: 18; radius: 3
                                            anchors.verticalCenter: parent.verticalCenter
                                            color: { switch(model.method) { case "GET": return "#4CAF50"; case "POST": return "#2196F3"; case "PUT": return "#FF9800"; case "DELETE": return "#F44336"; case "PATCH": return "#9C27B0"; default: return "#757575" } }
                                            UTText { anchors.centerIn: parent; text: model.method || ""; fontEnum: UIFontToken.Caption_Text; color: "white"; font.bold: true }
                                        }
                                        // Status
                                        UTText {
                                            width: proxyPanel.colStatusW; anchors.verticalCenter: parent.verticalCenter
                                            text: model.statusCode > 0 ? model.statusCode.toString() : "⋯"
                                            fontEnum: UIFontToken.Body_Text; font.family: "Consolas"
                                            color: { var c = model.statusCode||0; if(c>=200&&c<300) return "#4CAF50"; if(c>=300&&c<400) return "#FF9800"; if(c>=400) return "#F44336"; return proxyPanel._inputText }
                                        }
                                        // URL
                                        UTText {
                                            width: parent.width - proxyPanel.colMethodW - proxyPanel.colStatusW - proxyPanel.colTimeW - proxyPanel.colSizeW - proxyPanel.colProcessW - 5*5
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: (model.isHttps ? "🔒 " : "") + (model.host||"") + (model.path||"")
                                            fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight; font.family: "Consolas"
                                        }
                                        // Time
                                        UTText {
                                            width: proxyPanel.colTimeW; anchors.verticalCenter: parent.verticalCenter
                                            horizontalAlignment: Text.AlignRight; font.family: "Consolas"
                                            fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
                                            text: { var d=model.duration||0; if(d<=0) return "⋯"; if(d<1) return (d*1000).toFixed(0)+" ms"; return d.toFixed(2)+" s" }
                                        }
                                        // Size
                                        UTText {
                                            width: proxyPanel.colSizeW; anchors.verticalCenter: parent.verticalCenter
                                            horizontalAlignment: Text.AlignRight; font.family: "Consolas"
                                            fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
                                            text: { var l=model.contentLength||0; if(l<=0) return "⋯"; if(l<1024) return l+" B"; if(l<1048576) return (l/1024).toFixed(1)+" KB"; return (l/1048576).toFixed(1)+" MB" }
                                        }
                                        // Process
                                        UTText {
                                            width: proxyPanel.colProcessW; anchors.verticalCenter: parent.verticalCenter
                                            horizontalAlignment: Text.AlignLeft; font.family: "Consolas"
                                            fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
                                            text: model.processName || ""
                                            elide: Text.ElideRight
                                        }
                                    }
                                }

                                // Empty state
                                UTText {
                                    anchors.centerIn: parent; visible: requestListView.count === 0
                                    text: controller.proxyRunning ? qsTr("Waiting for requests...\nProxy: 127.0.0.1:%1").arg(controller.proxyPort) : qsTr("Click ▶ Start to begin capturing")
                                    fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.5; horizontalAlignment: Text.AlignHCenter
                                }
                            }
                        }
                    }

                    // ────── Right: Request + Response ──────
                    SplitView {
                        SplitView.fillWidth: true; SplitView.minimumWidth: 280
                        orientation: Qt.Vertical

                        // ─── Request Detail ───
                        Rectangle {
                            SplitView.preferredHeight: parent.height * 0.45; SplitView.minimumHeight: 80
                            color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4

                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 1; spacing: 0

                                // Tab bar
                                Rectangle {
                                    Layout.fillWidth: true; height: 36; color: proxyPanel._headerBg; radius: 3
                                    RowLayout {
                                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 4
                                        UTText { text: qsTr("Request"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                        Rectangle { width: 1; height: 18; color: proxyPanel._sectionBorder }
                                        Repeater {
                                            model: [qsTr("Headers"), qsTr("Body"), qsTr("Params")]
                                            delegate: Rectangle {
                                                width: reqTL.implicitWidth + 16; height: 26; radius: 3
                                                color: controller.requestTabIndex === index ? proxyPanel._accentColor : reqTM.containsMouse ? proxyPanel._headerHover : "transparent"
                                                UTText { id: reqTL; anchors.centerIn: parent; text: modelData; fontEnum: UIFontToken.Caption_Text; color: controller.requestTabIndex === index ? "white" : proxyPanel._sectionTitle; font.bold: controller.requestTabIndex === index }
                                                MouseArea { id: reqTM; anchors.fill: parent; hoverEnabled: true; onClicked: controller.requestTabIndex = index }
                                            }
                                        }
                                        Item { Layout.fillWidth: true }
                                        UTButton { text: qsTr("URL"); enabled: controller.selectedIndex >= 0; onClicked: controller.copyRequestUrl(); implicitHeight: 24; ToolTip.text: qsTr("Copy URL"); ToolTip.visible: hovered; ToolTip.delay: 500 }
                                        UTButton { text: qsTr("cURL"); enabled: controller.selectedIndex >= 0; onClicked: controller.copyRequestCurl(); implicitHeight: 24; ToolTip.text: qsTr("Copy as cURL"); ToolTip.visible: hovered; ToolTip.delay: 500 }
                                    }
                                }
                                // Content
                                ScrollView {
                                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                                    ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                                    TextArea {
                                        text: controller.requestDetailText; readOnly: true
                                        wrapMode: TextEdit.Wrap; font: proxyPanel._monoFont; color: proxyPanel._inputText
                                        padding: 6
                                        background: Rectangle { color: proxyPanel._inputBg; radius: 2 }
                                    }
                                }
                            }
                        }

                        // ─── Response Detail ───
                        Rectangle {
                            SplitView.fillHeight: true; SplitView.minimumHeight: 80
                            color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4

                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 1; spacing: 0
                                Rectangle {
                                    Layout.fillWidth: true; height: 36; color: proxyPanel._headerBg; radius: 3
                                    RowLayout {
                                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 4
                                        UTText { text: qsTr("Response"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                        Rectangle { width: 1; height: 18; color: proxyPanel._sectionBorder }
                                        Repeater {
                                            model: [qsTr("Headers"), qsTr("Body"), qsTr("Summary")]
                                            delegate: Rectangle {
                                                width: resTL.implicitWidth + 16; height: 26; radius: 3
                                                color: controller.responseTabIndex === index ? proxyPanel._accentColor : resTM.containsMouse ? proxyPanel._headerHover : "transparent"
                                                UTText { id: resTL; anchors.centerIn: parent; text: modelData; fontEnum: UIFontToken.Caption_Text; color: controller.responseTabIndex === index ? "white" : proxyPanel._sectionTitle; font.bold: controller.responseTabIndex === index }
                                                MouseArea { id: resTM; anchors.fill: parent; hoverEnabled: true; onClicked: controller.responseTabIndex = index }
                                            }
                                        }
                                        Item { Layout.fillWidth: true }
                                        UTButton { text: qsTr("Copy"); enabled: controller.selectedIndex >= 0; onClicked: controller.copyResponseBody(); implicitHeight: 24; ToolTip.text: qsTr("Copy response body"); ToolTip.visible: hovered; ToolTip.delay: 500 }
                                    }
                                }
                                ScrollView {
                                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                                    ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                                    TextArea {
                                        text: controller.responseDetailText; readOnly: true
                                        wrapMode: TextEdit.Wrap; font: proxyPanel._monoFont; color: proxyPanel._inputText
                                        padding: 6
                                        background: Rectangle { color: proxyPanel._inputBg; radius: 2 }
                                    }
                                }
                            }
                        }
                    }
                }
            } // end Tab 0

            // ╔══════════════════════════════════════╗
            // ║       TAB 1 — RULES                  ║
            // ╚══════════════════════════════════════╝
            Item {
                RowLayout {
                    anchors.fill: parent; spacing: 0

                    // ── Left nav ──
                    Rectangle {
                        Layout.preferredWidth: 140; Layout.fillHeight: true
                        color: proxyPanel._headerBg; radius: 4
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
                                    color: rulesNavRepeater.currentIndex === index ? Qt.alpha(proxyPanel._accentColor, 0.2) : rnMa.containsMouse ? proxyPanel._headerHover : "transparent"
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
                        color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4

                        StackLayout {
                            anchors.fill: parent; anchors.margins: 12
                            currentIndex: rulesNavRepeater.currentIndex

                            // ─── Mock Response ───
                            ColumnLayout {
                                spacing: 10
                                UTText { text: qsTr("Mock Response Rules"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                UTText { text: qsTr("Match requests by URL pattern and return a custom response. Mocked requests will appear in Capture with a \"MOCK\" tag."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                                // Form
                                GridLayout {
                                    columns: 4; columnSpacing: 8; rowSpacing: 6; Layout.fillWidth: true
                                    UTText { text: qsTr("URL Pattern:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                    ThemedInput { id: mockUrl; Layout.fillWidth: true; Layout.columnSpan: 2; placeholderText: qsTr("e.g. /api/user.*") }
                                    UTButton { text: qsTr("🧪 Test"); onClicked: { ptPattern.text = mockUrl.text; ptResult.text = ""; patternTestDialog.open() } }
                                    UTText { text: qsTr("Status:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                    ThemedInput { id: mockStatus; implicitWidth: 70; placeholderText: "200"; validator: IntValidator { bottom: 100; top: 599 } }
                                    UTText { text: qsTr("Content-Type:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                    ThemedInput { id: mockCt; implicitWidth: 180; placeholderText: "application/json"; text: "application/json" }
                                    UTText { text: qsTr("Headers:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                    ThemedInput { id: mockHeaders; Layout.fillWidth: true; Layout.columnSpan: 3; placeholderText: qsTr("X-Custom: value  (one per line or comma-separated)") }
                                    UTText { text: qsTr("Body:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                    ThemedInput { id: mockBody; Layout.fillWidth: true; Layout.columnSpan: 3; placeholderText: qsTr("{\"message\": \"mocked\"}") }
                                }
                                RowLayout {
                                    spacing: 8
                                    UTButton { text: qsTr("+ Add Rule"); onClicked: { controller.addMockRule(mockUrl.text, parseInt(mockStatus.text)||200, mockCt.text||"application/json", mockBody.text, mockHeaders.text); mockUrl.text=""; mockStatus.text=""; mockBody.text=""; mockHeaders.text=""; mockListView.model = controller.getMockRules() } }
                                    Item { Layout.fillWidth: true }
                                    UTButton { text: qsTr("Clear All"); onClicked: { controller.clearMockRules(); mockListView.model = controller.getMockRules() } }
                                }
                                // Rule list
                                Rectangle {
                                    Layout.fillWidth: true; Layout.fillHeight: true
                                    color: proxyPanel._inputBg; border.color: proxyPanel._inputBorder; border.width: 1; radius: 4
                                    ListView {
                                        id: mockListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                                        model: controller.getMockRules()
                                        delegate: Rectangle {
                                            width: mockListView.width; height: 32; radius: 3
                                            color: mockDelegateMa.containsMouse ? Qt.alpha(proxyPanel._accentColor, 0.12) : (index % 2 === 0 ? "transparent" : Qt.darker(proxyPanel._sectionBg, 1.03))
                                            MouseArea {
                                                id: mockDelegateMa; anchors.fill: parent; hoverEnabled: true
                                                onDoubleClicked: { mockDetailDialog.ruleData = modelData; mockDetailDialog.open() }
                                            }
                                            RowLayout {
                                                anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                                Rectangle { width: 40; height: 18; radius: 3; color: proxyPanel._accentColor; UTText { anchors.centerIn: parent; text: modelData.status_code || "200"; fontEnum: UIFontToken.Caption_Text; color: "white" } }
                                                UTText { Layout.fillWidth: true; text: modelData.url_pattern || ""; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight; font.family: "Consolas" }
                                                UTText { visible: (modelData.headers || "").length > 0; text: "📋"; font.pixelSize: 12; ToolTip.text: qsTr("Has custom headers"); ToolTip.visible: false }
                                                UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { controller.removeMockRule(index); mockListView.model = controller.getMockRules() } }
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

                                // Auto-intercept notice
                                Rectangle {
                                    Layout.fillWidth: true; height: 32; radius: 4; visible: !controller.interceptEnabled
                                    color: Qt.alpha("#FF9800", 0.1); border.color: Qt.alpha("#FF9800", 0.3); border.width: 1
                                    RowLayout {
                                        anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 6
                                        UTText { text: qsTr("⚠️ Interception is OFF — breakpoints won't take effect."); fontEnum: UIFontToken.Caption_Text; color: "#FF9800"; Layout.fillWidth: true }
                                        UTButton { text: qsTr("Enable"); implicitHeight: 24; onClicked: controller.interceptEnabled = true }
                                    }
                                }

                                RowLayout {
                                    spacing: 8; Layout.fillWidth: true
                                    ThemedInput { id: bpUrl; Layout.fillWidth: true; placeholderText: qsTr("URL pattern (regex)") }
                                    ComboBox { id: bpMethod; model: ["ANY","GET","POST","PUT","DELETE"]; implicitWidth: 100 }
                                    UTButton { text: qsTr("+ Add"); onClicked: { controller.addBreakpointRule(bpUrl.text, bpMethod.currentText); bpUrl.text=""; bpListView.model = controller.getBreakpointRules(); if (!controller.interceptEnabled) controller.interceptEnabled = true } }
                                    UTButton { text: qsTr("🧪 Test"); onClicked: { ptPattern.text = bpUrl.text; ptResult.text = ""; patternTestDialog.open() } }
                                    Item { Layout.fillWidth: true }
                                    UTButton { text: qsTr("Clear All"); onClicked: { controller.clearBreakpointRules(); bpListView.model = controller.getBreakpointRules() } }
                                }
                                Rectangle {
                                    Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 60
                                    color: proxyPanel._inputBg; border.color: proxyPanel._inputBorder; border.width: 1; radius: 4
                                    ListView {
                                        id: bpListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                                        model: controller.getBreakpointRules()
                                        delegate: Rectangle {
                                            width: bpListView.width; height: 28; radius: 3; color: index % 2 === 0 ? "transparent" : Qt.darker(proxyPanel._sectionBg, 1.03)
                                            RowLayout {
                                                anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                                Rectangle { width: 44; height: 18; radius: 3; color: "#9C27B0"; UTText { anchors.centerIn: parent; text: modelData.method || "ANY"; fontEnum: UIFontToken.Caption_Text; color: "white" } }
                                                UTText { Layout.fillWidth: true; text: modelData.url_pattern || ""; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight; font.family: "Consolas" }
                                                UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { controller.removeBreakpointRule(index); bpListView.model = controller.getBreakpointRules() } }
                                            }
                                        }
                                        UTText { anchors.centerIn: parent; visible: parent.count === 0; text: qsTr("No breakpoint rules."); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.5 }
                                    }
                                }

                                // ─── Paused Requests Panel ───
                                UTText {
                                    text: qsTr("Paused Requests (%1)").arg(interceptedFlowsModel.count)
                                    fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title
                                    visible: interceptedFlowsModel.count > 0
                                }
                                Rectangle {
                                    Layout.fillWidth: true; Layout.preferredHeight: Math.min(interceptedFlowsModel.count * 36 + 8, 180)
                                    visible: interceptedFlowsModel.count > 0
                                    color: Qt.alpha("#F44336", 0.05); border.color: Qt.alpha("#F44336", 0.3); border.width: 1; radius: 4
                                    ListView {
                                        id: pausedListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                                        model: interceptedFlowsModel
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
                                                UTText { Layout.fillWidth: true; text: model.url; fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideMiddle; font.family: "Consolas" }
                                                UTButton {
                                                    text: qsTr("▶ Forward"); implicitHeight: 24
                                                    onClicked: { controller.resumeRequest(model.flowId); interceptedFlowsModel.remove(index) }
                                                }
                                                UTButton {
                                                    text: qsTr("✕ Drop"); implicitHeight: 24
                                                    onClicked: { controller.dropRequest(model.flowId); interceptedFlowsModel.remove(index) }
                                                }
                                            }
                                        }
                                    }
                                }
                                // Forward All / Drop All buttons
                                RowLayout {
                                    visible: interceptedFlowsModel.count > 1; spacing: 8
                                    UTButton {
                                        text: qsTr("▶ Forward All (%1)").arg(interceptedFlowsModel.count)
                                        onClicked: {
                                            for (var i = interceptedFlowsModel.count - 1; i >= 0; i--)
                                                controller.resumeRequest(interceptedFlowsModel.get(i).flowId)
                                            interceptedFlowsModel.clear()
                                        }
                                    }
                                    UTButton {
                                        text: qsTr("✕ Drop All (%1)").arg(interceptedFlowsModel.count)
                                        onClicked: {
                                            for (var i = interceptedFlowsModel.count - 1; i >= 0; i--)
                                                controller.dropRequest(interceptedFlowsModel.get(i).flowId)
                                            interceptedFlowsModel.clear()
                                        }
                                    }
                                }

                                // Usage help text
                                Rectangle {
                                    Layout.fillWidth: true; implicitHeight: bpHelpCol.implicitHeight + 16; radius: 4
                                    color: Qt.alpha(proxyPanel._accentColor, 0.05); border.color: Qt.alpha(proxyPanel._accentColor, 0.2); border.width: 1
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
                                    ThemedInput { id: blUrl; Layout.fillWidth: true; placeholderText: qsTr("URL pattern to block (regex)") }
                                    UTButton { text: qsTr("+ Add"); onClicked: { controller.addBlacklistRule(blUrl.text); blUrl.text=""; blListView.model = controller.getBlacklistRules() } }
                                }
                                Rectangle {
                                    Layout.fillWidth: true; Layout.fillHeight: true
                                    color: proxyPanel._inputBg; border.color: proxyPanel._inputBorder; border.width: 1; radius: 4
                                    ListView {
                                        id: blListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                                        model: controller.getBlacklistRules()
                                        delegate: Rectangle {
                                            width: blListView.width; height: 28; radius: 3; color: index % 2 === 0 ? "transparent" : Qt.darker(proxyPanel._sectionBg, 1.03)
                                            RowLayout {
                                                anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                                Rectangle { width: 14; height: 14; radius: 7; color: "#F44336"; anchors.verticalCenter: parent.verticalCenter }
                                                UTText { Layout.fillWidth: true; text: modelData.url_pattern || modelData; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight; font.family: "Consolas" }
                                                UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { controller.removeBlacklistRule(index); blListView.model = controller.getBlacklistRules() } }
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
                                    ThemedInput { id: mlUrl; Layout.fillWidth: true; placeholderText: qsTr("URL pattern (regex)") }
                                    ThemedInput { id: mlPath; Layout.fillWidth: true; placeholderText: qsTr("Local file path") }
                                    UTButton { text: qsTr("+ Add"); onClicked: { controller.addMapLocalRule(mlUrl.text, mlPath.text); mlUrl.text=""; mlPath.text=""; mlListView.model = controller.getMapLocalRules() } }
                                }
                                Rectangle {
                                    Layout.fillWidth: true; Layout.fillHeight: true
                                    color: proxyPanel._inputBg; border.color: proxyPanel._inputBorder; border.width: 1; radius: 4
                                    ListView {
                                        id: mlListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                                        model: controller.getMapLocalRules()
                                        delegate: Rectangle {
                                            width: mlListView.width; height: 28; radius: 3; color: index % 2 === 0 ? "transparent" : Qt.darker(proxyPanel._sectionBg, 1.03)
                                            RowLayout {
                                                anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                                UTText { Layout.fillWidth: true; text: (modelData.url_pattern||"") + " → " + (modelData.local_path||""); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight; font.family: "Consolas" }
                                                UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { controller.removeMapLocalRule(index); mlListView.model = controller.getMapLocalRules() } }
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
                                    ThemedInput { id: mrSrc; Layout.fillWidth: true; placeholderText: qsTr("Source URL pattern (regex)") }
                                    ThemedInput { id: mrDest; Layout.fillWidth: true; placeholderText: qsTr("Destination URL") }
                                    UTButton { text: qsTr("+ Add"); onClicked: { controller.addMapRemoteRule(mrSrc.text, mrDest.text); mrSrc.text=""; mrDest.text=""; mrListView.model = controller.getMapRemoteRules() } }
                                }
                                Rectangle {
                                    Layout.fillWidth: true; Layout.fillHeight: true
                                    color: proxyPanel._inputBg; border.color: proxyPanel._inputBorder; border.width: 1; radius: 4
                                    ListView {
                                        id: mrListView; anchors.fill: parent; anchors.margins: 4; clip: true; spacing: 2
                                        model: controller.getMapRemoteRules()
                                        delegate: Rectangle {
                                            width: mrListView.width; height: 28; radius: 3; color: index % 2 === 0 ? "transparent" : Qt.darker(proxyPanel._sectionBg, 1.03)
                                            RowLayout {
                                                anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                                                UTText { Layout.fillWidth: true; text: (modelData.src_pattern||"") + " → " + (modelData.dest_url||""); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight; font.family: "Consolas" }
                                                UTButton { text: "✕"; implicitWidth: 28; implicitHeight: 24; onClicked: { controller.removeMapRemoteRule(index); mrListView.model = controller.getMapRemoteRules() } }
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

                                ThemedCheckBox { id: throttleEnabled; text: qsTr("Enable Throttle") }

                                GridLayout {
                                    columns: 3; columnSpacing: 12; rowSpacing: 8; Layout.fillWidth: true
                                    enabled: throttleEnabled.checked
                                    opacity: throttleEnabled.checked ? 1.0 : 0.5

                                    UTText { text: qsTr("Download:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                    Slider { id: dlSlider; from: 0; to: 10000; stepSize: 50; value: 0; Layout.fillWidth: true }
                                    UTText { text: dlSlider.value > 0 ? qsTr("%1 KB/s").arg(dlSlider.value) : qsTr("Unlimited"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; Layout.preferredWidth: 100 }

                                    UTText { text: qsTr("Upload:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                    Slider { id: ulSlider; from: 0; to: 10000; stepSize: 50; value: 0; Layout.fillWidth: true }
                                    UTText { text: ulSlider.value > 0 ? qsTr("%1 KB/s").arg(ulSlider.value) : qsTr("Unlimited"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; Layout.preferredWidth: 100 }
                                }

                                RowLayout {
                                    spacing: 12
                                    // Presets
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
                                        onClicked: controller.setThrottle(throttleEnabled.checked, dlSlider.value, ulSlider.value)
                                    }
                                }
                                Item { Layout.fillHeight: true }
                            }
                        }
                    }
                }
            } // end Tab 1

            // ╔══════════════════════════════════════╗
            // ║       TAB 2 — SETTINGS               ║
            // ╚══════════════════════════════════════╝
            Item {
                ScrollView {
                    anchors.fill: parent; clip: true

                    ColumnLayout {
                        width: parent.width; spacing: 16

                        // ── Proxy Settings ──
                        GroupBox {
                            Layout.fillWidth: true
                            title: qsTr("Proxy Settings")
                            font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
                            palette.windowText: proxyPanel._sectionTitle
                            background: Rectangle { y: parent.topPadding - parent.bottomPadding; width: parent.width; height: parent.height - parent.topPadding + parent.bottomPadding; color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4 }

                            GridLayout {
                                columns: 2; columnSpacing: 16; rowSpacing: 10; width: parent.width

                                UTText { text: qsTr("Proxy Port:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                RowLayout {
                                    spacing: 8
                                    ThemedInput {
                                        id: settingsPort; implicitWidth: 80
                                        text: controller.proxyPort.toString()
                                        validator: IntValidator { bottom: 1024; top: 65535 }
                                        onEditingFinished: controller.proxyPort = parseInt(text)
                                    }
                                    UTText { text: qsTr("(1024–65535)"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.6 }
                                }

                                UTText { text: qsTr("Control Port:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                RowLayout {
                                    spacing: 8
                                    ThemedInput {
                                        id: settingsCtlPort; implicitWidth: 80
                                        text: controller.controlPort.toString()
                                        validator: IntValidator { bottom: 1024; top: 65535 }
                                        onEditingFinished: controller.controlPort = parseInt(text)
                                    }
                                    UTText { text: qsTr("(addon TCP control port)"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.6 }
                                }

                                UTText { text: qsTr("System Proxy:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                ThemedCheckBox {
                                    id: settingsAutoProxy
                                    text: qsTr("Automatically configure system proxy on start/stop")
                                    checked: controller.autoSystemProxy
                                    onToggled: controller.autoSystemProxy = checked
                                }

                                UTText { text: qsTr("Intercept:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                ThemedCheckBox {
                                    text: qsTr("Enable request interception (requires breakpoint rules)")
                                    checked: controller.interceptEnabled
                                    onToggled: controller.interceptEnabled = checked
                                }
                            }
                        }

                        // ── Certificate ──
                        GroupBox {
                            Layout.fillWidth: true
                            title: qsTr("HTTPS Certificate")
                            font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
                            palette.windowText: proxyPanel._sectionTitle
                            background: Rectangle { y: parent.topPadding - parent.bottomPadding; width: parent.width; height: parent.height - parent.topPadding + parent.bottomPadding; color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4 }

                            ColumnLayout {
                                width: parent.width; spacing: 12

                                // Status banner
                                Rectangle {
                                    Layout.fillWidth: true; height: 52; radius: 6
                                    color: controller.caCertInstalled ? Qt.alpha("#4CAF50", 0.1) : Qt.alpha("#FF9800", 0.1)
                                    border.color: controller.caCertInstalled ? Qt.alpha("#4CAF50", 0.3) : Qt.alpha("#FF9800", 0.3)
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent; anchors.margins: 12; spacing: 10
                                        Text {
                                            text: controller.caCertInstalled ? "✅" : "⚠️"
                                            font.pixelSize: 22
                                        }
                                        ColumnLayout {
                                            spacing: 2; Layout.fillWidth: true
                                            UTText {
                                                text: controller.caCertInstalled
                                                    ? qsTr("Certificate installed and trusted")
                                                    : qsTr("Certificate not trusted")
                                                fontEnum: UIFontToken.Body_Text_Medium
                                                color: controller.caCertInstalled ? "#4CAF50" : "#FF9800"
                                            }
                                            UTText {
                                                text: controller.caCertInstalled
                                                    ? qsTr("HTTPS traffic capture is ready to use.")
                                                    : qsTr("HTTPS capture will show certificate errors. Install the CA cert below.")
                                                fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.7
                                            }
                                        }
                                    }
                                }

                                // One-click install section
                                ColumnLayout {
                                    visible: !controller.caCertInstalled
                                    Layout.fillWidth: true; spacing: 0

                                    Rectangle {
                                        Layout.fillWidth: true
                                        implicitHeight: installCol.implicitHeight + 24; radius: 6
                                        color: proxyPanel._inputBg; border.color: proxyPanel._inputBorder; border.width: 1

                                        ColumnLayout {
                                            id: installCol
                                            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                                            anchors.margins: 12; spacing: 10

                                            UTText {
                                                text: qsTr("Click the button below to install and trust the mitmproxy CA certificate. A system password dialog will appear — this is required to add the certificate to the trusted root store.")
                                                fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text
                                                wrapMode: Text.WordWrap; Layout.fillWidth: true
                                            }

                                            RowLayout {
                                                spacing: 12; Layout.alignment: Qt.AlignHCenter

                                                // Big install button
                                                Rectangle {
                                                    width: installBtnRow.implicitWidth + 32; height: 40; radius: 8
                                                    color: controller.certInstalling ? "#9E9E9E" : "#2196F3"
                                                    opacity: installBtnMa.containsMouse && !controller.certInstalling ? 0.85 : 1.0

                                                    RowLayout {
                                                        id: installBtnRow
                                                        anchors.centerIn: parent; spacing: 8

                                                        // Spinner animation when installing
                                                        Text {
                                                            visible: controller.certInstalling
                                                            text: "⏳"; font.pixelSize: 16
                                                            RotationAnimation on rotation { running: controller.certInstalling; from: 0; to: 360; duration: 1200; loops: Animation.Infinite }
                                                        }
                                                        Text {
                                                            visible: !controller.certInstalling
                                                            text: "🔐"; font.pixelSize: 16
                                                        }
                                                        Text {
                                                            text: controller.certInstalling ? qsTr("Installing...") : qsTr("One-Click Install CA Certificate")
                                                            color: "white"; font.pixelSize: 14; font.bold: true
                                                        }
                                                    }

                                                    MouseArea {
                                                        id: installBtnMa
                                                        anchors.fill: parent; hoverEnabled: true
                                                        cursorShape: controller.certInstalling ? Qt.WaitCursor : Qt.PointingHandCursor
                                                        onClicked: if (!controller.certInstalling) controller.installCACert()
                                                    }
                                                }
                                            }

                                            UTText {
                                                text: qsTr("Steps: Enter macOS password → Import to System Keychain → Auto-trust for SSL")
                                                fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
                                                wrapMode: Text.WordWrap; Layout.fillWidth: true; opacity: 0.5
                                            }
                                        }
                                    }
                                }

                                // File info & actions
                                RowLayout {
                                    spacing: 8; Layout.fillWidth: true
                                    UTText { text: qsTr("Path:"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text }
                                    UTText { text: controller.getCACertPath(); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; font.family: "Consolas"; elide: Text.ElideMiddle; Layout.fillWidth: true }
                                }

                                RowLayout {
                                    spacing: 8
                                    UTButton { text: qsTr("Show in Finder"); onClicked: controller.revealCACertInFolder() }
                                    UTButton { text: qsTr("Refresh Status"); onClicked: controller.caCertInstalledChanged() }
                                    UTButton { text: qsTr("Re-install"); visible: controller.caCertInstalled; onClicked: controller.installCACert(); enabled: !controller.certInstalling }
                                }
                            }
                        }

                        // ── About ──
                        GroupBox {
                            Layout.fillWidth: true
                            title: qsTr("About")
                            font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
                            palette.windowText: proxyPanel._sectionTitle
                            background: Rectangle { y: parent.topPadding - parent.bottomPadding; width: parent.width; height: parent.height - parent.topPadding + parent.bottomPadding; color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 4 }

                            GridLayout {
                                columns: 2; columnSpacing: 16; rowSpacing: 6; width: parent.width
                                UTText { text: qsTr("Engine:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                UTText { text: qsTr("mitmproxy (PyInstaller bundle)"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                UTText { text: qsTr("Communication:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                UTText { text: qsTr("TCP socket, newline-delimited JSON"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                UTText { text: qsTr("Proxy Address:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                                UTText { text: qsTr("127.0.0.1:%1").arg(controller.proxyPort); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.family: "Consolas" }
                            }
                        }
                        Item { Layout.fillHeight: true }
                    }
                }
            } // end Tab 2
        }
    }

    // ── Pattern Test Dialog ──
    Dialog {
        id: patternTestDialog
        title: qsTr("Test URL Pattern")
        anchors.centerIn: parent; width: 520; implicitHeight: ptDialogCol.implicitHeight + 100
        modal: true; standardButtons: Dialog.Close
        background: Rectangle { color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 8 }
        header: Rectangle {
            width: parent.width; height: 40; color: proxyPanel._headerBg; radius: 8
            UTText { anchors.centerIn: parent; text: qsTr("🧪 Regex Pattern Tester"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
        }
        ColumnLayout {
            id: ptDialogCol
            width: parent.width; spacing: 12
            GridLayout {
                columns: 2; columnSpacing: 10; rowSpacing: 8; Layout.fillWidth: true
                UTText { text: qsTr("Pattern:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                ThemedInput { id: ptPattern; Layout.fillWidth: true; placeholderText: qsTr("Regex pattern (e.g. /api/user.*)") }
                UTText { text: qsTr("Test URL:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                ThemedInput { id: ptTestUrl; Layout.fillWidth: true; placeholderText: qsTr("https://example.com/api/user/123") }
            }
            RowLayout {
                spacing: 8; Layout.alignment: Qt.AlignHCenter
                UTButton {
                    text: qsTr("▶ Run Test")
                    enabled: ptPattern.text.length > 0 && ptTestUrl.text.length > 0
                    onClicked: ptResult.text = controller.testUrlPattern(ptPattern.text, ptTestUrl.text)
                }
                UTButton {
                    text: qsTr("Clear")
                    onClicked: { ptResult.text = ""; ptTestUrl.text = "" }
                }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: ptResult.implicitHeight + 16; radius: 4
                visible: ptResult.text.length > 0
                color: ptResult.text.indexOf("✓") >= 0 ? Qt.alpha("#4CAF50", 0.1) : Qt.alpha("#F44336", 0.1)
                border.color: ptResult.text.indexOf("✓") >= 0 ? Qt.alpha("#4CAF50", 0.3) : Qt.alpha("#F44336", 0.3)
                border.width: 1
                UTText {
                    id: ptResult; anchors.centerIn: parent
                    fontEnum: UIFontToken.Body_Text; font.family: "Consolas"
                    color: text.indexOf("✓") >= 0 ? "#4CAF50" : "#F44336"
                    wrapMode: Text.WordWrap; width: parent.width - 16
                    horizontalAlignment: Text.AlignHCenter
                }
            }
            UTText {
                text: qsTr("Tip: Pattern uses Python-style regex (re.search). It matches anywhere in the URL, not just from the start.")
                fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.5
                wrapMode: Text.WordWrap; Layout.fillWidth: true
            }
        }
    }

    // ── Mock Rule Detail Dialog (double-click) ──
    Dialog {
        id: mockDetailDialog
        title: qsTr("Mock Rule Detail")
        anchors.centerIn: parent; width: 540; implicitHeight: mdDialogCol.implicitHeight + 100
        modal: true; standardButtons: Dialog.Close
        property var ruleData: ({})
        background: Rectangle { color: proxyPanel._sectionBg; border.color: proxyPanel._sectionBorder; border.width: 1; radius: 8 }
        header: Rectangle {
            width: parent.width; height: 40; color: proxyPanel._headerBg; radius: 8
            UTText { anchors.centerIn: parent; text: qsTr("🎭 Mock Rule Detail"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
        }
        ColumnLayout {
            id: mdDialogCol
            width: parent.width; spacing: 8
            GridLayout {
                columns: 2; columnSpacing: 10; rowSpacing: 6; Layout.fillWidth: true
                UTText { text: qsTr("URL Pattern:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
                UTText { text: mockDetailDialog.ruleData.url_pattern || ""; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.family: "Consolas"; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
                UTText { text: qsTr("Status Code:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
                UTText { text: String(mockDetailDialog.ruleData.status_code || 200); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                UTText { text: qsTr("Content-Type:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
                UTText { text: mockDetailDialog.ruleData.content_type || "application/json"; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                UTText { text: qsTr("Headers:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
                UTText { text: mockDetailDialog.ruleData.headers || "(none)"; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.family: "Consolas"; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
            }
            UTText { text: qsTr("Response Body:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: Math.max(80, mdBody.implicitHeight + 12); radius: 4
                color: proxyPanel._inputBg; border.color: proxyPanel._inputBorder; border.width: 1
                ScrollView {
                    anchors.fill: parent; clip: true
                    TextArea {
                        id: mdBody; readOnly: true; text: mockDetailDialog.ruleData.body || ""
                        font: proxyPanel._monoFont; color: proxyPanel._inputText; padding: 6; wrapMode: TextEdit.Wrap
                        background: null
                    }
                }
            }
            RowLayout {
                spacing: 8
                UTButton { text: qsTr("Edit (load into form)"); onClicked: {
                    mockUrl.text = mockDetailDialog.ruleData.url_pattern || ""
                    mockStatus.text = String(mockDetailDialog.ruleData.status_code || 200)
                    mockCt.text = mockDetailDialog.ruleData.content_type || "application/json"
                    mockBody.text = mockDetailDialog.ruleData.body || ""
                    mockHeaders.text = mockDetailDialog.ruleData.headers || ""
                    mockDetailDialog.close()
                    mainTabBar.currentIndex = 1; rulesNavRepeater.currentIndex = 0
                }}
                UTButton { text: qsTr("🧪 Test Pattern"); onClicked: {
                    ptPattern.text = mockDetailDialog.ruleData.url_pattern || ""
                    ptResult.text = ""
                    mockDetailDialog.close(); patternTestDialog.open()
                }}
            }
        }
    }

    FileDialog {
        id: exportDialog; title: qsTr("Export Requests"); fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]; onAccepted: controller.exportRequests(selectedFile)
    }
}
