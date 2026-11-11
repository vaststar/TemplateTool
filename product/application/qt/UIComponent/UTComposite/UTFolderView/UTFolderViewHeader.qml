import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTFolderViewHeader - Internal header for UTFolderView.
 *
 * Grid mode  : View-mode toggle buttons
 * Detail mode : Static column labels + view-mode toggle
 */
Item {
    id: root

    implicitHeight: viewMode === "detail" ? 64 : 36

    property string viewMode: "grid"
    property real scrollBarWidth: 0
    readonly property bool compact: width - 4 - scrollBarWidth < 520

    signal viewModeChangeRequested(string mode)

    // Keep the same toolbar and focus objects in both view modes.
    Row {
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.right: parent.right
        anchors.rightMargin: 8
        spacing: 8

        // View mode buttons
        UTToolButton {
            implicitWidth: 28; implicitHeight: 28
            focusPolicy: Qt.StrongFocus
            text: "\u25A6"
            toolTipText: qsTr("Grid View")
            backgroundColorEnum: root.viewMode === "grid" ? UIColorToken.Tab_Button_Checked_Background : UIColorToken.Tab_Button_Background
            fontColorEnum: root.viewMode === "grid" ? UIColorToken.Tab_Button_Checked_Text : UIColorToken.Tab_Button_Text
            onClicked: root.viewModeChangeRequested("grid")
            Keys.onReturnPressed: clicked()
            Keys.onEnterPressed: clicked()
        }
        UTToolButton {
            implicitWidth: 28; implicitHeight: 28
            focusPolicy: Qt.StrongFocus
            text: "\u2637"
            toolTipText: qsTr("Detail View")
            backgroundColorEnum: root.viewMode === "detail" ? UIColorToken.Tab_Button_Checked_Background : UIColorToken.Tab_Button_Background
            fontColorEnum: root.viewMode === "detail" ? UIColorToken.Tab_Button_Checked_Text : UIColorToken.Tab_Button_Text
            onClicked: root.viewModeChangeRequested("detail")
            Keys.onReturnPressed: clicked()
            Keys.onEnterPressed: clicked()
        }
    }

    // ─── Detail column headers (static labels) ───
    // Columns layer — margins match the data delegate exactly:
    //   ListView leftMargin(4) + bg leftMargin(4) + RowLayout leftMargin(8) = 16
    RowLayout {
        y: 36
        height: 28
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 16
        anchors.rightMargin: 12 + root.scrollBarWidth
        visible: root.viewMode === "detail"
        spacing: 0

        UTText {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.preferredWidth: 300
            text: qsTr("Name")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            elide: Text.ElideRight
        }

        UTText {
            visible: !root.compact
            Layout.minimumWidth: 160
            Layout.preferredWidth: 160
            Layout.maximumWidth: 160
            text: qsTr("Date Modified")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            leftPadding: 8
            elide: Text.ElideRight
        }

        UTText {
            Layout.minimumWidth: 80
            Layout.preferredWidth: 80
            Layout.maximumWidth: 80
            text: qsTr("Size")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            horizontalAlignment: Text.AlignRight
            leftPadding: 8
            elide: Text.ElideRight
        }

        UTText {
            visible: !root.compact
            Layout.minimumWidth: 70
            Layout.preferredWidth: 70
            Layout.maximumWidth: 70
            text: qsTr("Type")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            leftPadding: 8
            elide: Text.ElideRight
        }
    }
}
