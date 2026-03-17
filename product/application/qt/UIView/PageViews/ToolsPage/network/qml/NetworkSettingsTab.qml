import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root

    required property NetworkProxyController controller

    // ── Theme ──
    readonly property color _sectionBg:      UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder:   UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border,     UIColorState.Normal)
    readonly property color _sectionTitle:    UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Title,      UIColorState.Normal)
    readonly property color _inputBg:         UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background,   UIColorState.Normal)
    readonly property color _inputText:       UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text,         UIColorState.Normal)
    readonly property color _inputBorder:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Normal)
    readonly property color _inputPlaceholder:UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder,  UIColorState.Normal)
    readonly property color _accentColor:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Focused)
    readonly property font  _inputFont:       UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    readonly property font  _monoFont:        Qt.font({family: "Consolas", pixelSize: _inputFont.pixelSize})

    component ThemedInput : TextField {
        font: root._monoFont; color: root._inputText
        placeholderTextColor: root._inputPlaceholder
        background: Rectangle { color: root._inputBg; border.color: parent.activeFocus ? root._accentColor : root._inputBorder; border.width: 1; radius: 4 }
    }

    component ThemedCheckBox : CheckBox {
        font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)
        indicator: Rectangle {
            implicitWidth: 18; implicitHeight: 18; x: 2; y: parent.height / 2 - height / 2; radius: 3
            color: parent.checked ? root._accentColor : root._inputBg
            border.color: parent.checked ? root._accentColor : root._inputBorder; border.width: 1
            Text { anchors.centerIn: parent; text: "✓"; font.pixelSize: 13; font.bold: true; color: "white"; visible: parent.parent.checked }
        }
        contentItem: UTText {
            text: parent.text; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text
            leftPadding: parent.indicator.width + 6; verticalAlignment: Text.AlignVCenter
        }
    }

    ScrollView {
        anchors.fill: parent; clip: true

        ColumnLayout {
            width: parent.width; spacing: 16

            // ── Proxy Settings ──
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Proxy Settings")
                font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
                palette.windowText: root._sectionTitle
                background: Rectangle { y: parent.topPadding - parent.bottomPadding; width: parent.width; height: parent.height - parent.topPadding + parent.bottomPadding; color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4 }

                GridLayout {
                    columns: 2; columnSpacing: 16; rowSpacing: 10; width: parent.width

                    UTText { text: qsTr("Proxy Port:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    RowLayout {
                        spacing: 8
                        ThemedInput {
                            id: settingsPort; implicitWidth: 80
                            text: root.controller.proxyPort.toString()
                            validator: IntValidator { bottom: 1024; top: 65535 }
                            onEditingFinished: root.controller.proxyPort = parseInt(text)
                        }
                        UTText { text: qsTr("(1024–65535)"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.6 }
                    }

                    UTText { text: qsTr("Control Port:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    RowLayout {
                        spacing: 8
                        ThemedInput {
                            id: settingsCtlPort; implicitWidth: 80
                            text: root.controller.controlPort.toString()
                            validator: IntValidator { bottom: 1024; top: 65535 }
                            onEditingFinished: root.controller.controlPort = parseInt(text)
                        }
                        UTText { text: qsTr("(addon TCP control port)"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.6 }
                    }

                    UTText { text: qsTr("System Proxy:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    ThemedCheckBox {
                        text: qsTr("Automatically configure system proxy on start/stop")
                        checked: root.controller.autoSystemProxy
                        onToggled: root.controller.autoSystemProxy = checked
                    }

                    UTText { text: qsTr("Intercept:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    ThemedCheckBox {
                        text: qsTr("Enable request interception (requires breakpoint rules)")
                        checked: root.controller.interceptEnabled
                        onToggled: root.controller.interceptEnabled = checked
                    }
                }
            }

            // ── Certificate ──
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("HTTPS Certificate")
                font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
                palette.windowText: root._sectionTitle
                background: Rectangle { y: parent.topPadding - parent.bottomPadding; width: parent.width; height: parent.height - parent.topPadding + parent.bottomPadding; color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4 }

                ColumnLayout {
                    width: parent.width; spacing: 12

                    // Status banner
                    Rectangle {
                        Layout.fillWidth: true; height: 52; radius: 6
                        color: root.controller.certManager.caCertInstalled ? Qt.alpha("#4CAF50", 0.1) : Qt.alpha("#FF9800", 0.1)
                        border.color: root.controller.certManager.caCertInstalled ? Qt.alpha("#4CAF50", 0.3) : Qt.alpha("#FF9800", 0.3)
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent; anchors.margins: 12; spacing: 10
                            Text { text: root.controller.certManager.caCertInstalled ? "✅" : "⚠️"; font.pixelSize: 22 }
                            ColumnLayout {
                                spacing: 2; Layout.fillWidth: true
                                UTText {
                                    text: root.controller.certManager.caCertInstalled
                                        ? qsTr("Certificate installed and trusted")
                                        : qsTr("Certificate not trusted")
                                    fontEnum: UIFontToken.Body_Text_Medium
                                    color: root.controller.certManager.caCertInstalled ? "#4CAF50" : "#FF9800"
                                }
                                UTText {
                                    text: root.controller.certManager.caCertInstalled
                                        ? qsTr("HTTPS traffic capture is ready to use.")
                                        : qsTr("HTTPS capture will show certificate errors. Install the CA cert below.")
                                    fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.7
                                }
                            }
                        }
                    }

                    // One-click install section
                    ColumnLayout {
                        visible: !root.controller.certManager.caCertInstalled
                        Layout.fillWidth: true; spacing: 0

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: installCol.implicitHeight + 24; radius: 6
                            color: root._inputBg; border.color: root._inputBorder; border.width: 1

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

                                    Rectangle {
                                        width: installBtnRow.implicitWidth + 32; height: 40; radius: 8
                                        color: root.controller.certManager.certInstalling ? "#9E9E9E" : "#2196F3"
                                        opacity: installBtnMa.containsMouse && !root.controller.certManager.certInstalling ? 0.85 : 1.0

                                        RowLayout {
                                            id: installBtnRow; anchors.centerIn: parent; spacing: 8
                                            Text {
                                                visible: root.controller.certManager.certInstalling; text: "⏳"; font.pixelSize: 16
                                                RotationAnimation on rotation { running: root.controller.certManager.certInstalling; from: 0; to: 360; duration: 1200; loops: Animation.Infinite }
                                            }
                                            Text { visible: !root.controller.certManager.certInstalling; text: "🔐"; font.pixelSize: 16 }
                                            Text {
                                                text: root.controller.certManager.certInstalling ? qsTr("Installing...") : qsTr("One-Click Install CA Certificate")
                                                color: "white"; font.pixelSize: 14; font.bold: true
                                            }
                                        }

                                        MouseArea {
                                            id: installBtnMa; anchors.fill: parent; hoverEnabled: true
                                            cursorShape: root.controller.certManager.certInstalling ? Qt.WaitCursor : Qt.PointingHandCursor
                                            onClicked: if (!root.controller.certManager.certInstalling) root.controller.certManager.installCACert()
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
                        UTText { text: root.controller.certManager.getCACertPath(); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; font.family: "Consolas"; elide: Text.ElideMiddle; Layout.fillWidth: true }
                    }

                    RowLayout {
                        spacing: 8
                        UTButton { text: qsTr("Show in Finder"); onClicked: root.controller.certManager.revealCACertInFolder() }
                        UTButton { text: qsTr("Refresh Status"); onClicked: root.controller.certManager.caCertInstalledChanged() }
                        UTButton { text: qsTr("Re-install"); visible: root.controller.certManager.caCertInstalled; onClicked: root.controller.certManager.installCACert(); enabled: !root.controller.certManager.certInstalling }
                    }
                }
            }

            // ── About ──
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("About")
                font: UTComponentUtil.getUIFont(UIFontToken.Body_Text_Medium)
                palette.windowText: root._sectionTitle
                background: Rectangle { y: parent.topPadding - parent.bottomPadding; width: parent.width; height: parent.height - parent.topPadding + parent.bottomPadding; color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 4 }

                GridLayout {
                    columns: 2; columnSpacing: 16; rowSpacing: 6; width: parent.width
                    UTText { text: qsTr("Engine:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("mitmproxy (PyInstaller bundle)"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("Communication:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("TCP socket, newline-delimited JSON"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("Proxy Address:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("127.0.0.1:%1").arg(root.controller.proxyPort); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text; font.family: "Consolas" }
                }
            }
            Item { Layout.fillHeight: true }
        }
    }
}
