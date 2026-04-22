import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

UTDialog {
    id: dialog

    required property var controller

    title: qsTr("Software Update")
    width: 480
    height: 360
    minimumWidth: 400
    minimumHeight: 300

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        // Header
        UTText {
            text: {
                if (dialog.controller.readyToInstall)
                    return qsTr("Ready to install")
                if (dialog.controller.verifying)
                    return qsTr("Verifying package...")
                if (dialog.controller.downloading)
                    return qsTr("Downloading update...")
                if (dialog.controller.hasUpgrade)
                    return qsTr("A new version is available!")
                if (dialog.controller.checking)
                    return qsTr("Checking for updates...")
                return qsTr("Check for Updates")
            }
            fontEnum: UIFontToken.Heading_Text
            Layout.fillWidth: true
        }

        // Version info
        UTText {
            visible: dialog.controller.hasUpgrade
            text: qsTr("Version %1").arg(dialog.controller.version)
            fontEnum: UIFontToken.Body_Text
            Layout.fillWidth: true
        }

        // Release notes
        UTScrollView {
            visible: dialog.controller.hasUpgrade && dialog.controller.releaseNotes.length > 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            UTTextArea {
                readOnly: true
                text: dialog.controller.releaseNotes
                wrapMode: TextArea.Wrap
            }
        }

        // Checking / Verifying spinner
        BusyIndicator {
            visible: dialog.controller.checking || dialog.controller.verifying
            running: visible
            Layout.alignment: Qt.AlignHCenter
            palette.dark: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Text, UIColorState.Normal)
        }

        // Download progress
        ColumnLayout {
            visible: dialog.controller.downloading
            Layout.fillWidth: true
            spacing: 4

            ProgressBar {
                from: 0; to: 1.0
                value: dialog.controller.downloadProgress
                Layout.fillWidth: true
            }

            UTText {
                text: qsTr("%1%").arg(Math.round(dialog.controller.downloadProgress * 100))
                fontEnum: UIFontToken.Caption_Text
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // Error message
        UTText {
            visible: dialog.controller.errorMessage.length > 0
            text: dialog.controller.errorMessage
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Error_Text
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        // No update message
        UTText {
            visible: !dialog.controller.hasUpgrade && !dialog.controller.checking
                     && !dialog.controller.verifying && !dialog.controller.readyToInstall
                     && dialog.controller.errorMessage.length === 0
            text: qsTr("You are using the latest version.")
            fontEnum: UIFontToken.Body_Text
            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        // Buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Item { Layout.fillWidth: true }

            UTButton {
                visible: dialog.controller.downloading
                text: qsTr("Cancel")
                onClicked: dialog.controller.cancelDownload()
            }

            UTButton {
                visible: dialog.controller.readyToInstall
                text: qsTr("Install && Restart")
                onClicked: dialog.controller.installAndRestart()
            }

            UTButton {
                visible: dialog.controller.hasUpgrade && !dialog.controller.downloading
                         && !dialog.controller.verifying && !dialog.controller.readyToInstall
                text: qsTr("Download && Install")
                onClicked: dialog.controller.downloadUpgrade()
            }

            UTButton {
                visible: !dialog.controller.downloading && !dialog.controller.verifying
                text: dialog.controller.hasUpgrade ? qsTr("Later") : qsTr("Close")
                fontColorEnum: dialog.controller.hasUpgrade ? UIColorToken.Content_Text : UIColorToken.Button_Primary_Text
                backgroundColorEnum: dialog.controller.hasUpgrade ? UIColorToken.Content_Background : UIColorToken.Button_Primary_Background
                borderColorEnum: dialog.controller.hasUpgrade ? UIColorToken.Content_Input_Border : UIColorToken.Button_Primary_Border
                onClicked: dialog.close()
            }
        }
    }

    onClosing: {
        dialog.controller.dismiss()
    }
}
