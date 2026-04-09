import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTFolderGridDelegate - Grid card delegate for UTFolderView.
 *
 * Displays a thumbnail, file name, and modification date in a card layout.
 * The thumbnail area is customisable via thumbnailComponent.
 */
Item {
    id: root

    property Component thumbnailComponent: null
    property bool isSelected: false
    property bool isCurrent: false

    // ── Theme colors ──
    readonly property color _bgNormal:   UTComponentUtil.getPlainUIColor(UIColorToken.Listview_Item_Background, UIColorState.Normal)
    readonly property color _bgHovered:  UTComponentUtil.getPlainUIColor(UIColorToken.Listview_Item_Background, UIColorState.Hovered)
    readonly property color _bgSelected: UTComponentUtil.getPlainUIColor(UIColorToken.Listview_Item_Background, UIColorState.Selected)
    readonly property color _textNormal:   UTComponentUtil.getPlainUIColor(UIColorToken.Listview_Item_Text, UIColorState.Normal)
    readonly property color _textSelected: UTComponentUtil.getPlainUIColor(UIColorToken.Listview_Item_Text, UIColorState.Selected)
    readonly property color _secondaryText: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Secondary_Text, UIColorState.Normal)

    signal clicked(int mouseButton)
    signal doubleClicked()

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 4
        radius: 8
        color: root.isSelected ? root._bgSelected : (_mouseArea.containsMouse ? root._bgHovered : root._bgNormal)
        border.width: 0

        UTFocusItem {
            delegateFocused: root.isCurrent
            focusRadius: card.radius
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 4

            // Thumbnail area (customisable)
            Loader {
                Layout.fillWidth: true
                Layout.fillHeight: true
                sourceComponent: root.thumbnailComponent ?? _defaultThumbnail

                property url fileUrl: model.fileUrl
                property string fileName: model.fileName
                property string filePath: model.filePath
            }

            UTText {
                Layout.fillWidth: true
                text: model.fileName
                font.pixelSize: 11
                color: root.isSelected ? root._textSelected : root._textNormal
                elide: Text.ElideMiddle
                horizontalAlignment: Text.AlignHCenter
            }

            UTText {
                Layout.fillWidth: true
                text: Qt.formatDateTime(model.fileModified, "yyyy-MM-dd hh:mm")
                font.pixelSize: 10
                color: root.isSelected ? root._textNormal : root._secondaryText
                horizontalAlignment: Text.AlignHCenter
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
    }

    // Default thumbnail: Image with loading indicator
    Component {
        id: _defaultThumbnail
        Item {
            Image {
                anchors.fill: parent
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
                }
            }
        }
    }
}
