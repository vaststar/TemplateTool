import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * CameraDeleteDialog - Modal delete-confirmation for the camera directory tree.
 *
 * A standalone window spawned by CameraMonitorViewController via createQmlWindow().
 * The controller injects itself plus the target id; this view only confirms and
 * forwards the removal. Centering is handled by the C++ side.
 */
UTDialog {
    id: dialog

    // Injected via createWithInitialProperties() by the controller.
    required property var controller
    required property string targetId

    title: qsTr("Delete")
    width: 360
    height: contentColumn.implicitHeight + 48
    minimumWidth: 320
    minimumHeight: 140

    Shortcut {
        sequence: "Escape"
        onActivated: dialog.close()
    }

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        UTText {
            text: qsTr("Delete this item? Its children will be moved to the root.")
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Text
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            UTButton {
                focus: true
                text: qsTr("Cancel")
                appearance: UTButton.Secondary
                implicitWidth: 72
                implicitHeight: 30
                onClicked: dialog.close()
            }
            UTButton {
                text: qsTr("Delete")
                appearance: UTButton.Destructive
                implicitWidth: 72
                implicitHeight: 30
                onClicked: {
                    dialog.controller.removeNode(dialog.targetId);
                    dialog.close();
                }
            }
        }
    }
}
