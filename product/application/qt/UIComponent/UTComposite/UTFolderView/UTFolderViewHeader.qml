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

    implicitHeight: 36

    property string viewMode: "grid"
    property real scrollBarWidth: 0

    signal viewModeChangeRequested(string mode)

    // ─── Grid / List header ───
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        visible: root.viewMode !== "detail"
        spacing: 8

        Item { Layout.fillWidth: true }

        // View mode buttons
        UTToolButton {
            implicitWidth: 28; implicitHeight: 28
            text: "\u25A6"
            toolTipText: qsTr("Grid View")
            backgroundColorEnum: root.viewMode === "grid" ? UIColorToken.Tab_Button_Checked_Background : UIColorToken.Tab_Button_Background
            fontColorEnum: root.viewMode === "grid" ? UIColorToken.Tab_Button_Checked_Text : UIColorToken.Tab_Button_Text
            onClicked: root.viewModeChangeRequested("grid")
        }
        UTToolButton {
            implicitWidth: 28; implicitHeight: 28
            text: "\u2637"
            toolTipText: qsTr("Detail View")
            backgroundColorEnum: root.viewMode === "detail" ? UIColorToken.Tab_Button_Checked_Background : UIColorToken.Tab_Button_Background
            fontColorEnum: root.viewMode === "detail" ? UIColorToken.Tab_Button_Checked_Text : UIColorToken.Tab_Button_Text
            onClicked: root.viewModeChangeRequested("detail")
        }
    }

    // ─── Detail column headers (static labels) ───
    // Columns layer — margins match the data delegate exactly:
    //   ListView leftMargin(4) + bg leftMargin(4) + RowLayout leftMargin(8) = 16
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 12 + root.scrollBarWidth
        visible: root.viewMode === "detail"
        spacing: 0

        UTText {
            Layout.fillWidth: true
            Layout.preferredWidth: 300
            text: qsTr("Name")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
        }

        UTText {
            Layout.preferredWidth: 160
            text: qsTr("Date Modified")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            leftPadding: 8
        }

        UTText {
            Layout.preferredWidth: 80
            text: qsTr("Size")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            leftPadding: 8
        }

        UTText {
            Layout.preferredWidth: 70
            text: qsTr("Type")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            leftPadding: 8
        }
    }

    // View mode buttons (floating overlay, right-aligned)
    Row {
        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        visible: root.viewMode === "detail"
        spacing: 0

        UTToolButton {
            implicitWidth: 24; implicitHeight: 24
            text: "\u25A6"
            backgroundColorEnum: UIColorToken.Tab_Button_Background
            fontColorEnum: UIColorToken.Tab_Button_Text
            onClicked: root.viewModeChangeRequested("grid")
        }
        UTToolButton {
            implicitWidth: 24; implicitHeight: 24
            text: "\u2637"
            backgroundColorEnum: UIColorToken.Tab_Button_Checked_Background
            fontColorEnum: UIColorToken.Tab_Button_Checked_Text
            onClicked: {} // already in detail
        }
    }
}
