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
    readonly property color _inputBg:         UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background,   UIColorState.Normal)
    readonly property color _inputBorder:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Normal)

    UTScrollView {
        anchors.fill: parent

        ColumnLayout {
            width: parent.width; spacing: 16

            // ── Proxy Settings ──
            UTGroupBox {
                Layout.fillWidth: true
                title: qsTr("Proxy Settings")

                contentItem: GridLayout {
                    columns: 2; columnSpacing: 16; rowSpacing: 10

                    UTText { text: qsTr("Proxy Port:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    RowLayout {
                        spacing: 8
                        UTTextField {
                            id: settingsPort; implicitWidth: 80
                            fontEnum: UIFontToken.Monospace_Text
                            text: root.controller.proxyPort.toString()
                            validator: IntValidator { bottom: 1024; top: 65535 }
                            onEditingFinished: root.controller.proxyPort = parseInt(text)
                        }
                        UTText { text: qsTr("(1024–65535)"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Secondary_Text }
                    }

                    UTText { text: qsTr("System Proxy:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTCheckBox {
                        text: qsTr("Automatically configure system proxy on start/stop")
                        checked: root.controller.autoSystemProxy
                        onToggled: root.controller.autoSystemProxy = checked
                    }

                    UTText { text: qsTr("Intercept:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTCheckBox {
                        text: qsTr("Enable request interception (requires breakpoint rules)")
                        checked: root.controller.interceptEnabled
                        onToggled: root.controller.interceptEnabled = checked
                    }
                }
            }

            // ── Certificate ──
            UTGroupBox {
                Layout.fillWidth: true
                title: qsTr("HTTPS Certificate")

                contentItem: ColumnLayout {
                    spacing: 12

                    // Status banner
                    Rectangle {
                        Layout.fillWidth: true; height: 52; radius: 6
                        color: root.controller.caCertInstalled ? Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Status_Success_Background, UIColorState.Normal), 0.1) : Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Status_Warning_Background, UIColorState.Normal), 0.1)
                        border.color: root.controller.caCertInstalled ? Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Status_Success_Background, UIColorState.Normal), 0.3) : Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Status_Warning_Background, UIColorState.Normal), 0.3)
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent; anchors.margins: 12; spacing: 10
                            Text { text: root.controller.caCertInstalled ? "✅" : "⚠️"; font.pixelSize: 22 } // emoji glyph, not tokenized
                            ColumnLayout {
                                spacing: 2; Layout.fillWidth: true
                                UTText {
                                    text: root.controller.caCertInstalled
                                        ? qsTr("Certificate installed and trusted")
                                        : qsTr("Certificate not trusted")
                                    fontEnum: UIFontToken.Body_Text_Medium
                                    color: root.controller.caCertInstalled ? UTComponentUtil.getPlainUIColor(UIColorToken.Status_Success_Text, UIColorState.Normal) : UTComponentUtil.getPlainUIColor(UIColorToken.Status_Warning_Text, UIColorState.Normal)
                                }
                                UTText {
                                    text: root.controller.caCertInstalled
                                        ? qsTr("HTTPS traffic capture is ready to use.")
                                        : qsTr("HTTPS capture will show certificate errors. Install the CA cert below.")
                                    fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Secondary_Text
                                }
                            }
                        }
                    }

                    // One-click install section
                    ColumnLayout {
                        visible: !root.controller.caCertInstalled
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

                                    UTButton {
                                        appearance: UTButton.Primary
                                        id: installButton
                                        implicitWidth: installBtnRow.implicitWidth + 32
                                        implicitHeight: 40
                                        radius: 8
                                        borderWidth: 0
                                        padding: 0
                                        leftPadding: 16
                                        rightPadding: 16
                                        fontEnum: UIFontToken.Body_Text_Medium
                                        text: root.controller.certInstalling ? qsTr("Installing...") : qsTr("One-Click Install CA Certificate")
                                        enabled: !root.controller.certInstalling
                                        onClicked: root.controller.installCACert()

                                        contentItem: RowLayout {
                                            id: installBtnRow; spacing: 8
                                            Text {
                                                visible: root.controller.certInstalling; text: "⏳"; font.pixelSize: 16 // emoji glyph, not tokenized
                                                RotationAnimation on rotation { running: root.controller.certInstalling; from: 0; to: 360; duration: 1200; loops: Animation.Infinite }
                                            }
                                            Text { visible: !root.controller.certInstalling; text: "🔐"; font.pixelSize: 16 } // emoji glyph, not tokenized
                                            Text {
                                                text: installButton.text
                                                color: installButton.textColor
                                                font: installButton.font
                                            }
                                        }
                                    }
                                }

                                UTText {
                                    text: qsTr("Steps: Enter macOS password → Import to System Keychain → Auto-trust for SSL")
                                    fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Secondary_Text
                                    wrapMode: Text.WordWrap; Layout.fillWidth: true
                                }
                            }
                        }
                    }

                    // File info & actions
                    RowLayout {
                        spacing: 8; Layout.fillWidth: true
                        UTText { text: qsTr("Path:"); fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text }
                        UTText { text: root.controller.caCertPath; fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text; elide: Text.ElideMiddle; Layout.fillWidth: true }
                    }

                    RowLayout {
                        spacing: 8
                        UTButton { appearance: UTButton.Secondary; text: qsTr("Show in Finder"); onClicked: root.controller.revealCACertInFolder() }
                        UTButton { appearance: UTButton.Secondary; text: qsTr("Refresh Status"); onClicked: root.controller.checkCertStatus() }
                        UTButton { appearance: UTButton.Secondary; text: qsTr("Re-install"); visible: root.controller.caCertInstalled; onClicked: root.controller.installCACert(); enabled: !root.controller.certInstalling }
                    }
                }
            }

            // ── About ──
            UTGroupBox {
                Layout.fillWidth: true
                title: qsTr("About")

                contentItem: GridLayout {
                    columns: 2; columnSpacing: 16; rowSpacing: 6
                    UTText { text: qsTr("Engine:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("mitmproxy (PyInstaller bundle)"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("Communication:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("TCP socket, newline-delimited JSON"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("Proxy Address:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    UTText { text: qsTr("127.0.0.1:%1").arg(root.controller.proxyPort); fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text }
                }
            }
            Item { Layout.fillHeight: true }
        }
    }
}
