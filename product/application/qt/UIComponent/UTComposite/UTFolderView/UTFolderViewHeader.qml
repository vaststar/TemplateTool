import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTFolderViewHeader - Internal header for UTFolderView.
 *
 * Grid/List mode : Sort dropdown + asc/desc button + view-mode toggle
 * Detail mode    : Clickable column headers with sort indicators
 */
Item {
    id: root

    implicitHeight: 36

    property string viewMode: "grid"
    property int sortField: FolderListModel.Time
    property bool sortAscending: true

    signal viewModeChangeRequested(string mode)
    signal sortChangeRequested(int field, bool ascending)

    // ─── Grid / List header ───
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        visible: root.viewMode !== "detail"
        spacing: 8

        UTText {
            text: qsTr("Sort:")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
        }

        ComboBox {
            id: sortCombo
            implicitWidth: 120
            model: [
                { text: qsTr("Name"), value: FolderListModel.Name },
                { text: qsTr("Date"), value: FolderListModel.Time },
                { text: qsTr("Size"), value: FolderListModel.Size },
                { text: qsTr("Type"), value: FolderListModel.Type }
            ]
            textRole: "text"
            valueRole: "value"
            currentIndex: {
                for (var i = 0; i < model.length; i++) {
                    if (model[i].value === root.sortField) return i
                }
                return 1
            }
            onActivated: function(idx) {
                root.sortChangeRequested(model[idx].value, root.sortAscending)
            }
        }

        ToolButton {
            implicitWidth: 28
            implicitHeight: 28
            contentItem: Text {
                text: root.sortAscending ? "↑" : "↓"
                font.pixelSize: 14
                font.bold: true
                color: "#CCCCCC"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                radius: 4
                color: parent.hovered ? "#3A3A3A" : "transparent"
            }
            onClicked: root.sortChangeRequested(root.sortField, !root.sortAscending)
            ToolTip.visible: hovered
            ToolTip.text: root.sortAscending ? qsTr("Ascending") : qsTr("Descending")
        }

        Item { Layout.fillWidth: true }

        // View mode buttons
        ToolButton {
            implicitWidth: 28; implicitHeight: 28
            contentItem: Text {
                text: "\u25A6"; font.pixelSize: 14
                color: root.viewMode === "grid" ? "#FFFFFF" : "#888888"
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                radius: 4
                color: root.viewMode === "grid" ? "#4488FF" : (parent.hovered ? "#3A3A3A" : "transparent")
            }
            onClicked: root.viewModeChangeRequested("grid")
            ToolTip.visible: hovered; ToolTip.text: qsTr("Grid View")
        }
        ToolButton {
            implicitWidth: 28; implicitHeight: 28
            contentItem: Text {
                text: "\u2630"; font.pixelSize: 14
                color: root.viewMode === "list" ? "#FFFFFF" : "#888888"
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                radius: 4
                color: root.viewMode === "list" ? "#4488FF" : (parent.hovered ? "#3A3A3A" : "transparent")
            }
            onClicked: root.viewModeChangeRequested("list")
            ToolTip.visible: hovered; ToolTip.text: qsTr("List View")
        }
        ToolButton {
            implicitWidth: 28; implicitHeight: 28
            contentItem: Text {
                text: "\u2637"; font.pixelSize: 14
                color: root.viewMode === "detail" ? "#FFFFFF" : "#888888"
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                radius: 4
                color: root.viewMode === "detail" ? "#4488FF" : (parent.hovered ? "#3A3A3A" : "transparent")
            }
            onClicked: root.viewModeChangeRequested("detail")
            ToolTip.visible: hovered; ToolTip.text: qsTr("Detail View")
        }
    }

    // ─── Detail column headers ───
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        visible: root.viewMode === "detail"
        spacing: 0

        // Name column
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: _nameMA.containsMouse ? "#2A2A2A" : "transparent"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 4
                UTText {
                    text: qsTr("Name")
                    fontEnum: UIFontToken.Caption_Text
                    color: "#CCCCCC"
                    font.bold: true
                }
                UTText {
                    visible: root.sortField === FolderListModel.Name
                    text: root.sortAscending ? "▲" : "▼"
                    font.pixelSize: 10
                    color: "#CCCCCC"
                }
            }
            MouseArea {
                id: _nameMA; anchors.fill: parent; hoverEnabled: true
                onClicked: root.sortChangeRequested(FolderListModel.Name,
                               root.sortField === FolderListModel.Name ? !root.sortAscending : true)
            }
        }

        // Date column
        Rectangle {
            Layout.preferredWidth: 160
            Layout.fillHeight: true
            color: _dateMA.containsMouse ? "#2A2A2A" : "transparent"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 4
                UTText {
                    text: qsTr("Date Modified")
                    fontEnum: UIFontToken.Caption_Text
                    color: "#CCCCCC"
                    font.bold: true
                }
                UTText {
                    visible: root.sortField === FolderListModel.Time
                    text: root.sortAscending ? "▲" : "▼"
                    font.pixelSize: 10
                    color: "#CCCCCC"
                }
            }
            MouseArea {
                id: _dateMA; anchors.fill: parent; hoverEnabled: true
                onClicked: root.sortChangeRequested(FolderListModel.Time,
                               root.sortField === FolderListModel.Time ? !root.sortAscending : true)
            }
        }

        // Size column
        Rectangle {
            Layout.preferredWidth: 80
            Layout.fillHeight: true
            color: _sizeMA.containsMouse ? "#2A2A2A" : "transparent"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 4
                UTText {
                    text: qsTr("Size")
                    fontEnum: UIFontToken.Caption_Text
                    color: "#CCCCCC"
                    font.bold: true
                }
                UTText {
                    visible: root.sortField === FolderListModel.Size
                    text: root.sortAscending ? "▲" : "▼"
                    font.pixelSize: 10
                    color: "#CCCCCC"
                }
            }
            MouseArea {
                id: _sizeMA; anchors.fill: parent; hoverEnabled: true
                onClicked: root.sortChangeRequested(FolderListModel.Size,
                               root.sortField === FolderListModel.Size ? !root.sortAscending : true)
            }
        }

        // Type column
        Rectangle {
            Layout.preferredWidth: 70
            Layout.fillHeight: true
            color: _typeMA.containsMouse ? "#2A2A2A" : "transparent"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 4
                UTText {
                    text: qsTr("Type")
                    fontEnum: UIFontToken.Caption_Text
                    color: "#CCCCCC"
                    font.bold: true
                }
                UTText {
                    visible: root.sortField === FolderListModel.Type
                    text: root.sortAscending ? "▲" : "▼"
                    font.pixelSize: 10
                    color: "#CCCCCC"
                }
            }
            MouseArea {
                id: _typeMA; anchors.fill: parent; hoverEnabled: true
                onClicked: root.sortChangeRequested(FolderListModel.Type,
                               root.sortField === FolderListModel.Type ? !root.sortAscending : true)
            }
        }

        Item { Layout.preferredWidth: 8 }

        // View mode toggle (also in detail header)
        ToolButton {
            implicitWidth: 24; implicitHeight: 24
            contentItem: Text {
                text: "\u25A6"; font.pixelSize: 12; color: "#888888"
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle { radius: 4; color: parent.hovered ? "#3A3A3A" : "transparent" }
            onClicked: root.viewModeChangeRequested("grid")
        }
        ToolButton {
            implicitWidth: 24; implicitHeight: 24
            contentItem: Text {
                text: "\u2630"; font.pixelSize: 12; color: "#888888"
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle { radius: 4; color: parent.hovered ? "#3A3A3A" : "transparent" }
            onClicked: root.viewModeChangeRequested("list")
        }
        ToolButton {
            implicitWidth: 24; implicitHeight: 24
            contentItem: Text {
                text: "\u2637"; font.pixelSize: 12; color: "#FFFFFF"
                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle { radius: 4; color: "#4488FF" }
            onClicked: {} // already in detail
        }
    }
}
