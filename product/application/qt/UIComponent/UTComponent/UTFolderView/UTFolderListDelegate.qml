import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTFolderListDelegate - List row delegate for UTFolderView.
 *
 * Displays a thumbnail, file name, date, and file size in a horizontal row.
 * The thumbnail area is customisable via thumbnailComponent.
 */
Rectangle {
    id: root

    height: 56
    radius: 6
    color: root.isSelected ? "#2A3A5A" : (_mouseArea.containsMouse ? "#333333" : "transparent")
    border.width: 0

    property Component thumbnailComponent: null
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
        anchors.rightMargin: 12
        spacing: 12

        // Thumbnail (customisable)
        Rectangle {
            width: 44
            height: 44
            radius: 4
            color: "#2A2A2A"

            Loader {
                anchors.fill: parent
                anchors.margins: 2
                sourceComponent: root.thumbnailComponent ?? _defaultThumbnail

                property url fileUrl: model.fileUrl
                property string fileName: model.fileName
                property string filePath: model.filePath
            }
        }

        // File info
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            UTText {
                text: model.fileName
                font.pixelSize: 13
                color: root.isSelected ? "#FFFFFF" : "#DDDDDD"
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }

            UTText {
                text: Qt.formatDateTime(model.fileModified, "yyyy-MM-dd hh:mm:ss")
                font.pixelSize: 11
                color: root.isSelected ? "#BBBBBB" : "#888888"
            }
        }

        // File size
        UTText {
            text: {
                var s = model.fileSize
                if (s === undefined || s === 0) return ""
                if (s > 1048576) return (s / 1048576).toFixed(1) + " MB"
                return (s / 1024).toFixed(0) + " KB"
            }
            font.pixelSize: 11
            color: "#888888"
            Layout.preferredWidth: 60
            horizontalAlignment: Text.AlignRight
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

    // Default thumbnail
    Component {
        id: _defaultThumbnail
        Image {
            source: fileUrl
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            cache: true
            smooth: true
            mipmap: true

            BusyIndicator {
                anchors.centerIn: parent
                running: parent.status === Image.Loading
                visible: running
                width: 20
                height: 20
            }
        }
    }
}
