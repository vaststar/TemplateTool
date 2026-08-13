import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * ContactDeleteDialog - Modal delete-confirmation for the contact tree.
 *
 * A standalone window spawned by ContactsPageController via createQmlWindow().
 * The controller injects itself plus the target id; this view only confirms and
 * forwards the removal. Centering is handled by the C++ side.
 */
UTDialog {
    id: dialog

    // Injected via createWithInitialProperties() by the controller.
    required property var controller
    required property string targetId

    title: "删除"
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
            text: "确定删除该项？其子项将移动到根目录。"
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Text
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            UTButton {
                text: "取消"
                backgroundColorEnum: UIColorToken.Content_Background
                borderColorEnum: UIColorToken.Content_Input_Border
                fontColorEnum: UIColorToken.Content_Text
                implicitWidth: 72
                implicitHeight: 30
                onClicked: dialog.close()
            }
            UTButton {
                text: "删除"
                implicitWidth: 72
                implicitHeight: 30
                onClicked: {
                    dialog.controller.removeContact(dialog.targetId);
                    dialog.close();
                }
            }
        }
    }
}
