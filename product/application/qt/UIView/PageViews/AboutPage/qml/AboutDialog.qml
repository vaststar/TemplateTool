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
    width: 440
    height: contentColumn.implicitHeight + 48
    minimumWidth: 380
    minimumHeight: 300

    onClosing: {
        controller.dialogClosed()
    }

    // ---- Internal helpers --------------------------------------------------
    component InfoRow : RowLayout {
        property string label
        property string value
        visible: value.length > 0
        Layout.fillWidth: true
        spacing: 12

        UTText {
            text: label
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            Layout.preferredWidth: 96
            horizontalAlignment: Text.AlignRight
        }
        UTText {
            text: value
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Text
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
    }
    // -----------------------------------------------------------------------

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // Header: product name + version subtitle, centered.
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            UTText {
                text: dialog.controller.productName
                fontEnum: UIFontToken.Heading_Text
                colorEnum: UIColorToken.Content_Text
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            UTText {
                text: qsTr("Version %1").arg(dialog.controller.version)
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Content_Secondary_Text
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
        }

        // Description (optional).
        UTText {
            visible: dialog.controller.description.length > 0
            text: dialog.controller.description
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Text
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        // Visual separator before the key-value table.
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Window_Border, UIColorState.Normal)
            opacity: 0.4
        }

        // Info table: label / value rows.
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            InfoRow { label: qsTr("Qt Version");  value: dialog.controller.qtVersion }
            InfoRow { label: qsTr("Company");     value: dialog.controller.companyName }
            InfoRow { label: qsTr("Copyright");   value: dialog.controller.copyright }
        }

        Item { Layout.fillHeight: true }

        // Footer: right-aligned OK button (system convention).
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight

            Item { Layout.fillWidth: true }

            UTButton {
                text: qsTr("OK")
                onClicked: dialog.close()
            }
        }
    }
}
