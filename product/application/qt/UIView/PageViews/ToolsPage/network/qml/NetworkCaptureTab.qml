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

    signal goToBreakpoints()

    // ── Theme ──
    readonly property color _inputBg:         UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background,   UIColorState.Normal)
    readonly property color _inputText:       UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text,         UIColorState.Normal)
    readonly property color _inputBorder:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Normal)
    readonly property color _inputPlaceholder:UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder,  UIColorState.Normal)
    readonly property color _sectionBg:      UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder:   UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border,     UIColorState.Normal)
    readonly property color _sectionTitle:    UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Title,      UIColorState.Normal)
    readonly property font  _inputFont:       UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    readonly property font  _monoFont:        Qt.font({family: "Consolas", pixelSize: _inputFont.pixelSize})
    readonly property color _headerBg:        Qt.darker(_sectionBg, 1.08)
    readonly property color _headerHover:     Qt.darker(_sectionBg, 1.15)
    readonly property color _accentColor:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Focused)

    // ── Draggable column widths ──
    property real colMethodW: 64
    property real colStatusW: 56
    property real colTimeW:   65
    property real colSizeW:   72
    property real colProcessW: 90

    // ── Method multi-select filter ──
    property var selectedMethods: []
    function methodFilterString() { return selectedMethods.length === 0 ? "ALL" : selectedMethods.join(",") }
    function toggleMethod(m) { var a = selectedMethods.slice(); var i = a.indexOf(m); if (i >= 0) a.splice(i, 1); else a.push(m); selectedMethods = a }
    function isMethodSelected(m) { return selectedMethods.length === 0 || selectedMethods.indexOf(m) >= 0 }

    // ── Status multi-select filter ──
    property var selectedStatuses: []
    function statusFilterString() { return selectedStatuses.length === 0 ? "ALL" : selectedStatuses.join(",") }
    function toggleStatus(s) { var a = selectedStatuses.slice(); var i = a.indexOf(s); if (i >= 0) a.splice(i, 1); else a.push(s); selectedStatuses = a }

    // ── Process multi-select filter ──
    property var selectedProcesses: []
    function processFilterString() { return selectedProcesses.length === 0 ? "ALL" : selectedProcesses.join(",") }
    function toggleProcess(p) { var a = selectedProcesses.slice(); var i = a.indexOf(p); if (i >= 0) a.splice(i, 1); else a.push(p); selectedProcesses = a }

    // ── Column drag handle component ──
    component ColDragHandle : Rectangle {
        property string targetCol
        width: 5; height: parent.height
        color: handleMa.containsMouse ? root._accentColor : "transparent"; radius: 1
        MouseArea {
            id: handleMa; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.SplitHCursor
            property real startX: 0; property real startW: 0
            onPressed: function(mouse) {
                startX = mapToGlobal(mouse.x, 0).x
                switch (targetCol) {
                case "method":  startW = root.colMethodW;  break; case "status":  startW = root.colStatusW;  break
                case "time":    startW = root.colTimeW;    break; case "size":    startW = root.colSizeW;    break
                case "process": startW = root.colProcessW; break
                }
            }
            onPositionChanged: function(mouse) {
                if (!pressed) return
                var dx = mapToGlobal(mouse.x, 0).x - startX
                var leftEdge = (targetCol === "time" || targetCol === "size" || targetCol === "process")
                var nw = Math.max(36, startW + (leftEdge ? -dx : dx))
                switch (targetCol) {
                case "method":  root.colMethodW  = nw; break; case "status":  root.colStatusW  = nw; break
                case "time":    root.colTimeW    = nw; break; case "size":    root.colSizeW    = nw; break
                case "process": root.colProcessW = nw; break
                }
            }
        }
    }

    ProxyFilterModel {
        id: filterModel
        sourceModel: root.controller.requestModel
        filterUrl: filterInput.text
        filterMethod: root.methodFilterString()
        filterStatus: root.statusFilterString()
        filterProcess: root.processFilterString()
    }

    // ── Paused requests notification banner ──
    Rectangle {
        id: pausedBanner
        visible: root.interceptedFlowsModel.count > 0
        z: 10; anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right
        height: visible ? 32 : 0; radius: 4
        color: Qt.alpha("#F44336", 0.12); border.color: Qt.alpha("#F44336", 0.4); border.width: 1
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 6
            Text { text: "🔴"; font.pixelSize: 12 }
            UTText {
                text: qsTr("%1 request(s) paused by breakpoint").arg(root.interceptedFlowsModel.count)
                fontEnum: UIFontToken.Caption_Text; color: "#F44336"; font.bold: true; Layout.fillWidth: true
            }
            UTButton {
                text: qsTr("Go to Breakpoints"); implicitHeight: 24
                onClicked: root.goToBreakpoints()
            }
            UTButton {
                text: qsTr("Forward All"); implicitHeight: 24
                onClicked: {
                    for (var i = root.interceptedFlowsModel.count - 1; i >= 0; i--)
                        root.controller.resumeRequest(root.interceptedFlowsModel.get(i).flowId)
                    root.interceptedFlowsModel.clear()
                }
            }
        }
    }

    SplitView {
        anchors.fill: parent; anchors.topMargin: pausedBanner.visible ? 36 : 0
        orientation: Qt.Horizontal

        // ────── Left: Capture List ──────
        Rectangle {
            SplitView.preferredWidth: root.width * 0.48; SplitView.minimumWidth: 320
            color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 1; spacing: 0

                // ─── Column Headers with drag handles ───
                Rectangle {
                    Layout.fillWidth: true; height: 30; color: root._headerBg; radius: 3

                    Row {
                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8

                        // METHOD header
                        Rectangle {
                            width: root.colMethodW; height: parent.height
                            color: methodHeaderMa.containsMouse ? root._headerHover : "transparent"; radius: 2
                            Row {
                                anchors.centerIn: parent; spacing: 2
                                UTText { text: qsTr("Method"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                UTText {
                                    text: root.selectedMethods.length > 0 ? "▾" : "▿"; fontEnum: UIFontToken.Caption_Text
                                    color: root.selectedMethods.length > 0 ? root._accentColor : root._sectionTitle
                                }
                            }
                            MouseArea { id: methodHeaderMa; anchors.fill: parent; hoverEnabled: true; onClicked: methodPopup.open() }
                            Popup {
                                id: methodPopup; y: parent.height + 2; width: 160; padding: 4
                                background: Rectangle { color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4 }
                                Column {
                                    width: parent.width; spacing: 1
                                    Rectangle {
                                        width: parent.width; height: 26
                                        color: allMa.containsMouse ? root._headerHover : "transparent"; radius: 3
                                        Row {
                                            anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                            UTText { text: qsTr("ALL"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.bold: root.selectedMethods.length === 0; anchors.verticalCenter: parent.verticalCenter }
                                        }
                                        UTText { anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; visible: root.selectedMethods.length === 0; text: "✓"; color: root._accentColor; fontEnum: UIFontToken.Body_Text }
                                        MouseArea { id: allMa; anchors.fill: parent; hoverEnabled: true; onClicked: root.selectedMethods = [] }
                                    }
                                    Repeater {
                                        model: ["GET","POST","PUT","DELETE","PATCH","OPTIONS","HEAD"]
                                        delegate: Rectangle {
                                            width: parent.width; height: 26
                                            color: mhMa.containsMouse ? root._headerHover : "transparent"; radius: 3
                                            Row {
                                                anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                                Rectangle {
                                                    width: 14; height: 14; radius: 2; anchors.verticalCenter: parent.verticalCenter
                                                    color: root.selectedMethods.indexOf(modelData) >= 0 ? root._accentColor : "transparent"
                                                    border.color: root.selectedMethods.indexOf(modelData) >= 0 ? root._accentColor : root._inputBorder; border.width: 1
                                                    Text { anchors.centerIn: parent; text: "✓"; font.pixelSize: 10; color: "white"; visible: root.selectedMethods.indexOf(modelData) >= 0 }
                                                }
                                                Rectangle {
                                                    width: 6; height: 6; radius: 3; anchors.verticalCenter: parent.verticalCenter
                                                    color: { switch(modelData) { case "GET": return "#4CAF50"; case "POST": return "#2196F3"; case "PUT": return "#FF9800"; case "DELETE": return "#F44336"; case "PATCH": return "#9C27B0"; default: return "#757575" } }
                                                }
                                                UTText { text: modelData; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; anchors.verticalCenter: parent.verticalCenter }
                                            }
                                            MouseArea { id: mhMa; anchors.fill: parent; hoverEnabled: true; onClicked: root.toggleMethod(modelData) }
                                        }
                                    }
                                }
                            }
                        }
                        ColDragHandle { targetCol: "method" }

                        // STATUS header
                        Rectangle {
                            width: root.colStatusW; height: parent.height
                            color: statusHeaderMa.containsMouse ? root._headerHover : "transparent"; radius: 2
                            Row {
                                anchors.centerIn: parent; spacing: 2
                                UTText { text: qsTr("Status"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                UTText {
                                    text: root.selectedStatuses.length > 0 ? "▾" : "▿"; fontEnum: UIFontToken.Caption_Text
                                    color: root.selectedStatuses.length > 0 ? root._accentColor : root._sectionTitle
                                }
                            }
                            MouseArea { id: statusHeaderMa; anchors.fill: parent; hoverEnabled: true; onClicked: statusPopup.open() }
                            Popup {
                                id: statusPopup; y: parent.height + 2; width: 130; padding: 4
                                background: Rectangle { color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4 }
                                Column {
                                    width: parent.width; spacing: 1
                                    Rectangle {
                                        width: parent.width; height: 26
                                        color: allStatusMa.containsMouse ? root._headerHover : "transparent"; radius: 3
                                        Row {
                                            anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                            UTText { text: qsTr("ALL"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.bold: root.selectedStatuses.length === 0; anchors.verticalCenter: parent.verticalCenter }
                                        }
                                        UTText { anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; visible: root.selectedStatuses.length === 0; text: "✓"; color: root._accentColor; fontEnum: UIFontToken.Body_Text }
                                        MouseArea { id: allStatusMa; anchors.fill: parent; hoverEnabled: true; onClicked: root.selectedStatuses = [] }
                                    }
                                    Repeater {
                                        model: ["2xx","3xx","4xx","5xx"]
                                        delegate: Rectangle {
                                            width: parent.width; height: 26
                                            color: shMa.containsMouse ? root._headerHover : "transparent"; radius: 3
                                            Row {
                                                anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                                Rectangle {
                                                    width: 14; height: 14; radius: 2; anchors.verticalCenter: parent.verticalCenter
                                                    color: root.selectedStatuses.indexOf(modelData) >= 0 ? root._accentColor : "transparent"
                                                    border.color: root.selectedStatuses.indexOf(modelData) >= 0 ? root._accentColor : root._inputBorder; border.width: 1
                                                    Text { anchors.centerIn: parent; text: "✓"; font.pixelSize: 10; color: "white"; visible: root.selectedStatuses.indexOf(modelData) >= 0 }
                                                }
                                                Rectangle {
                                                    width: 6; height: 6; radius: 3; anchors.verticalCenter: parent.verticalCenter
                                                    color: { switch(modelData) { case "2xx": return "#4CAF50"; case "3xx": return "#FF9800"; case "4xx": return "#F44336"; case "5xx": return "#D32F2F"; default: return "#757575" } }
                                                }
                                                UTText { text: modelData; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; anchors.verticalCenter: parent.verticalCenter }
                                            }
                                            MouseArea { id: shMa; anchors.fill: parent; hoverEnabled: true; onClicked: root.toggleStatus(modelData) }
                                        }
                                    }
                                }
                            }
                        }
                        ColDragHandle { targetCol: "status" }

                        // URL header + inline filter
                        Item {
                            width: parent.width - root.colMethodW - root.colStatusW - root.colTimeW - root.colSizeW - root.colProcessW - 5*5
                            height: parent.height
                            Row {
                                anchors.fill: parent; spacing: 4; anchors.verticalCenter: parent.verticalCenter
                                UTText { text: qsTr("URL"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title; anchors.verticalCenter: parent.verticalCenter }
                                TextField {
                                    id: filterInput
                                    width: parent.width - 40; height: 22; anchors.verticalCenter: parent.verticalCenter
                                    placeholderText: qsTr("Filter..."); placeholderTextColor: root._inputPlaceholder
                                    font: Qt.font({family: "Consolas", pixelSize: root._inputFont.pixelSize - 2}); color: root._inputText
                                    leftPadding: 4; rightPadding: 4; topPadding: 2; bottomPadding: 2
                                    background: Rectangle { color: root._inputBg; border.color: filterInput.activeFocus ? root._accentColor : root._inputBorder; border.width: 1; radius: 3 }
                                }
                            }
                        }
                        ColDragHandle { targetCol: "time" }

                        // TIME header
                        Item {
                            width: root.colTimeW; height: parent.height
                            UTText { anchors.centerIn: parent; text: qsTr("Time"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                        }
                        ColDragHandle { targetCol: "size" }

                        // SIZE header
                        Item {
                            width: root.colSizeW; height: parent.height
                            UTText { anchors.centerIn: parent; text: qsTr("Size"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                        }
                        ColDragHandle { targetCol: "process" }

                        // PROCESS header
                        Rectangle {
                            width: root.colProcessW; height: parent.height
                            color: processHeaderMa.containsMouse ? root._headerHover : "transparent"; radius: 2
                            Row {
                                anchors.centerIn: parent; spacing: 2
                                UTText { text: qsTr("Process"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                                UTText {
                                    text: root.selectedProcesses.length > 0 ? "▾" : "▿"; fontEnum: UIFontToken.Caption_Text
                                    color: root.selectedProcesses.length > 0 ? root._accentColor : root._sectionTitle
                                }
                            }
                            MouseArea { id: processHeaderMa; anchors.fill: parent; hoverEnabled: true; onClicked: { processListModel.refresh(); processPopup.open() } }

                            ListModel {
                                id: processListModel
                                function refresh() {
                                    clear()
                                    var names = root.controller.requestModel.uniqueProcessNames()
                                    for (var i = 0; i < names.length; i++) append({"name": names[i]})
                                }
                            }

                            Popup {
                                id: processPopup; y: parent.height + 2; width: 200; padding: 4
                                background: Rectangle { color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4 }
                                Column {
                                    width: parent.width; spacing: 1
                                    Rectangle {
                                        width: parent.width; height: 26
                                        color: allProcMa.containsMouse ? root._headerHover : "transparent"; radius: 3
                                        Row {
                                            anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                            UTText { text: qsTr("ALL"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.bold: root.selectedProcesses.length === 0; anchors.verticalCenter: parent.verticalCenter }
                                        }
                                        UTText { anchors.right: parent.right; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; visible: root.selectedProcesses.length === 0; text: "✓"; color: root._accentColor; fontEnum: UIFontToken.Body_Text }
                                        MouseArea { id: allProcMa; anchors.fill: parent; hoverEnabled: true; onClicked: root.selectedProcesses = [] }
                                    }
                                    ListView {
                                        width: parent.width; height: Math.min(processListModel.count * 26, 200)
                                        clip: true; model: processListModel
                                        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                                        delegate: Rectangle {
                                            width: parent ? parent.width : 0; height: 26
                                            color: phMa.containsMouse ? root._headerHover : "transparent"; radius: 3
                                            Row {
                                                anchors.fill: parent; anchors.leftMargin: 8; spacing: 6; anchors.verticalCenter: parent.verticalCenter
                                                Rectangle {
                                                    width: 14; height: 14; radius: 2; anchors.verticalCenter: parent.verticalCenter
                                                    color: root.selectedProcesses.indexOf(model.name) >= 0 ? root._accentColor : "transparent"
                                                    border.color: root.selectedProcesses.indexOf(model.name) >= 0 ? root._accentColor : root._inputBorder; border.width: 1
                                                    Text { anchors.centerIn: parent; text: "✓"; font.pixelSize: 10; color: "white"; visible: root.selectedProcesses.indexOf(model.name) >= 0 }
                                                }
                                                UTText { text: model.name; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; anchors.verticalCenter: parent.verticalCenter; elide: Text.ElideRight; width: parent.width - 32 }
                                            }
                                            MouseArea { id: phMa; anchors.fill: parent; hoverEnabled: true; onClicked: root.toggleProcess(model.name) }
                                        }
                                    }
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
                    currentIndex: root.controller.selectedIndex
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    delegate: Rectangle {
                        width: requestListView.width; height: 26
                        color: index === requestListView.currentIndex
                            ? Qt.alpha(root._accentColor, 0.2)
                            : (index % 2 === 0 ? "transparent" : Qt.darker(root._sectionBg, 1.03))

                        MouseArea {
                            anchors.fill: parent
                            onClicked: { requestListView.currentIndex = index; root.controller.selectedIndex = filterModel.mapToSource(filterModel.index(index, 0)).row }
                        }

                        Row {
                            anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 5

                            Rectangle {
                                visible: model.isIntercepted === true
                                width: 18; height: 18; radius: 9; anchors.verticalCenter: parent.verticalCenter; color: "#F44336"
                                Text { anchors.centerIn: parent; text: "⏸"; font.pixelSize: 10; color: "white" }
                            }
                            Rectangle {
                                width: root.colMethodW - 5; height: 18; radius: 3; anchors.verticalCenter: parent.verticalCenter
                                color: { switch(model.method) { case "GET": return "#4CAF50"; case "POST": return "#2196F3"; case "PUT": return "#FF9800"; case "DELETE": return "#F44336"; case "PATCH": return "#9C27B0"; default: return "#757575" } }
                                UTText { anchors.centerIn: parent; text: model.method || ""; fontEnum: UIFontToken.Caption_Text; color: "white"; font.bold: true }
                            }
                            UTText {
                                width: root.colStatusW; anchors.verticalCenter: parent.verticalCenter
                                text: model.statusCode > 0 ? model.statusCode.toString() : "⋯"
                                fontEnum: UIFontToken.Body_Text; font.family: "Consolas"
                                color: { var c = model.statusCode||0; if(c>=200&&c<300) return "#4CAF50"; if(c>=300&&c<400) return "#FF9800"; if(c>=400) return "#F44336"; return root._inputText }
                            }
                            UTText {
                                width: parent.width - root.colMethodW - root.colStatusW - root.colTimeW - root.colSizeW - root.colProcessW - 5*5
                                anchors.verticalCenter: parent.verticalCenter
                                text: (model.isHttps ? "🔒 " : "") + (model.host||"") + (model.path||"")
                                fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideRight; font.family: "Consolas"
                            }
                            UTText {
                                width: root.colTimeW; anchors.verticalCenter: parent.verticalCenter
                                horizontalAlignment: Text.AlignRight; font.family: "Consolas"
                                fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
                                text: { var d=model.duration||0; if(d<=0) return "⋯"; if(d<1) return (d*1000).toFixed(0)+" ms"; return d.toFixed(2)+" s" }
                            }
                            UTText {
                                width: root.colSizeW; anchors.verticalCenter: parent.verticalCenter
                                horizontalAlignment: Text.AlignRight; font.family: "Consolas"
                                fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
                                text: { var l=model.contentLength||0; if(l<=0) return "⋯"; if(l<1024) return l+" B"; if(l<1048576) return (l/1024).toFixed(1)+" KB"; return (l/1048576).toFixed(1)+" MB" }
                            }
                            UTText {
                                width: root.colProcessW; anchors.verticalCenter: parent.verticalCenter
                                horizontalAlignment: Text.AlignLeft; font.family: "Consolas"
                                fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text
                                text: model.processName || ""; elide: Text.ElideRight
                            }
                        }
                    }

                    UTText {
                        anchors.centerIn: parent; visible: requestListView.count === 0
                        text: root.controller.proxyRunning ? qsTr("Waiting for requests...\nProxy: 127.0.0.1:%1").arg(root.controller.proxyPort) : qsTr("Click ▶ Start to begin capturing")
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
                color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 1; spacing: 0
                    Rectangle {
                        Layout.fillWidth: true; height: 36; color: root._headerBg; radius: 3
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 4
                            UTText { text: qsTr("Request"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                            Rectangle { width: 1; height: 18; color: root._sectionBorder }
                            Repeater {
                                model: [qsTr("Headers"), qsTr("Body"), qsTr("Params")]
                                delegate: Rectangle {
                                    width: reqTL.implicitWidth + 16; height: 26; radius: 3
                                    color: root.controller.requestTabIndex === index ? root._accentColor : reqTM.containsMouse ? root._headerHover : "transparent"
                                    UTText { id: reqTL; anchors.centerIn: parent; text: modelData; fontEnum: UIFontToken.Caption_Text; color: root.controller.requestTabIndex === index ? "white" : root._sectionTitle; font.bold: root.controller.requestTabIndex === index }
                                    MouseArea { id: reqTM; anchors.fill: parent; hoverEnabled: true; onClicked: root.controller.requestTabIndex = index }
                                }
                            }
                            Item { Layout.fillWidth: true }
                            UTButton { text: qsTr("URL"); enabled: root.controller.selectedIndex >= 0; onClicked: root.controller.copyRequestUrl(); implicitHeight: 24; ToolTip.text: qsTr("Copy URL"); ToolTip.visible: hovered; ToolTip.delay: 500 }
                            UTButton { text: qsTr("cURL"); enabled: root.controller.selectedIndex >= 0; onClicked: root.controller.copyRequestCurl(); implicitHeight: 24; ToolTip.text: qsTr("Copy as cURL"); ToolTip.visible: hovered; ToolTip.delay: 500 }
                        }
                    }
                    ScrollView {
                        Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                        ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                        TextArea {
                            text: root.controller.requestDetailText; readOnly: true
                            wrapMode: TextEdit.Wrap; font: root._monoFont; color: root._inputText; padding: 6
                            background: Rectangle { color: root._inputBg; radius: 2 }
                        }
                    }
                }
            }

            // ─── Response Detail ───
            Rectangle {
                SplitView.fillHeight: true; SplitView.minimumHeight: 80
                color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 1; spacing: 0
                    Rectangle {
                        Layout.fillWidth: true; height: 36; color: root._headerBg; radius: 3
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 4
                            UTText { text: qsTr("Response"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                            Rectangle { width: 1; height: 18; color: root._sectionBorder }
                            Repeater {
                                model: [qsTr("Headers"), qsTr("Body"), qsTr("Summary")]
                                delegate: Rectangle {
                                    width: resTL.implicitWidth + 16; height: 26; radius: 3
                                    color: root.controller.responseTabIndex === index ? root._accentColor : resTM.containsMouse ? root._headerHover : "transparent"
                                    UTText { id: resTL; anchors.centerIn: parent; text: modelData; fontEnum: UIFontToken.Caption_Text; color: root.controller.responseTabIndex === index ? "white" : root._sectionTitle; font.bold: root.controller.responseTabIndex === index }
                                    MouseArea { id: resTM; anchors.fill: parent; hoverEnabled: true; onClicked: root.controller.responseTabIndex = index }
                                }
                            }
                            Item { Layout.fillWidth: true }
                            UTButton { text: qsTr("Copy"); enabled: root.controller.selectedIndex >= 0; onClicked: root.controller.copyResponseBody(); implicitHeight: 24; ToolTip.text: qsTr("Copy response body"); ToolTip.visible: hovered; ToolTip.delay: 500 }
                        }
                    }
                    ScrollView {
                        Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                        ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                        TextArea {
                            text: root.controller.responseDetailText; readOnly: true
                            wrapMode: TextEdit.Wrap; font: root._monoFont; color: root._inputText; padding: 6
                            background: Rectangle { color: root._inputBg; radius: 2 }
                        }
                    }
                }
            }
        }
    }
}
