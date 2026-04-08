import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTFolderDetailDelegate - Detail/table row delegate for UTFolderView.
 *
 * Compact row showing icon, name, date, size, and type.
 * Column widths match UTFolderViewHeader detail columns.
 */
Rectangle {
    id: root

    height: 28
    radius: 3
    color: root.isSelected ? "#2A3A5A" : (_mouseArea.containsMouse ? "#2A2A2A" : "transparent")
    border.width: 0

    property Component iconComponent: null
    property bool isSelected: false
    property bool isCurrent: false

    signal clicked(int mouseButton)
    signal doubleClicked()

    UTFocusItem {
        delegateFocused: root.isCurrent
        focusRadius: root.radius
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 0

        // Icon + Name column (flex: fills remaining)
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 300
            spacing: 6

            Loader {
                width: 18
                height: 18
                sourceComponent: root.iconComponent ?? _defaultIcon

                property url fileUrl: model.fileUrl
                property string fileName: model.fileName
            }

            UTText {
                Layout.fillWidth: true
                text: model.fileName
                font.pixelSize: 12
                color: root.isSelected ? "#FFFFFF" : "#DDDDDD"
                elide: Text.ElideRight
            }
        }

        // Date column
        UTText {
            Layout.preferredWidth: 160
            text: Qt.formatDateTime(model.fileModified, "yyyy-MM-dd hh:mm:ss")
            font.pixelSize: 11
            color: root.isSelected ? "#BBBBBB" : "#888888"
            leftPadding: 8
        }

        // Size column
        UTText {
            Layout.preferredWidth: 80
            text: {
                var s = model.fileSize
                if (s === undefined || s === 0) return ""
                if (s > 1048576) return (s / 1048576).toFixed(1) + " MB"
                if (s > 1024) return (s / 1024).toFixed(0) + " KB"
                return s + " B"
            }
            font.pixelSize: 11
            color: "#888888"
            horizontalAlignment: Text.AlignRight
            leftPadding: 8
        }

        // Type column
        UTText {
            Layout.preferredWidth: 70
            text: {
                var ext = model.fileName.split('.').pop()
                return ext ? ext.toUpperCase() : ""
            }
            font.pixelSize: 11
            color: "#888888"
            leftPadding: 8
        }
    }

    MouseArea {
        id: _mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: function(mouse) { root.clicked(mouse.button) }
        onDoubleClicked: root.doubleClicked()
    }

    // Default icon
    Component {
        id: _defaultIcon
        Text {
            text: "📄"
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
