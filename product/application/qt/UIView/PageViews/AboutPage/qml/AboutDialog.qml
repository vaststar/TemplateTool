import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0
import UIView 1.0

UTDialog {
    id: dialog

    property AboutPageController controller: AboutPageController {}

    title: qsTr("About")
    width: 400
    height: 300
    minimumWidth: 360
    minimumHeight: 260

    onClosing: {
        controller.dialogClosed()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        // Product name
        UTText {
            text: dialog.controller.productName
            fontEnum: UIFontToken.Heading_Text
            colorEnum: UIColorToken.Content_Text
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        // Version
        UTText {
            text: qsTr("Version %1").arg(dialog.controller.version)
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        // Description
        UTText {
            visible: dialog.controller.description.length > 0
            text: dialog.controller.description
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Text
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Item { Layout.fillHeight: true }

        // Copyright & company
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            UTText {
                text: dialog.controller.companyName
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Secondary_Text
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            UTText {
                visible: dialog.controller.copyright.length > 0
                text: dialog.controller.copyright
                fontEnum: UIFontToken.Caption_Text
                colorEnum: UIColorToken.Content_Secondary_Text
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
        }

        // Close button
        UTButton {
            text: qsTr("OK")
            Layout.alignment: Qt.AlignHCenter
            onClicked: dialog.close()
        }
    }
}
