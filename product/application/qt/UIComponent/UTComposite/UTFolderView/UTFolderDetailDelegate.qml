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
Item {
    id: root

    height: 28
    z: root.isCurrent ? 1 : 0

    property Component iconComponent: null
    property bool isSelected: false
    property bool isCurrent: false
    property real scrollBarWidth: 0
    property bool compact: width - scrollBarWidth < 520

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
        id: bg
        anchors.fill: parent
        anchors.leftMargin: 4
        // Mirrored margins reserve the scrollbar on the correct side in RTL.
        anchors.rightMargin: 4 + root.scrollBarWidth
        radius: 3
        color: root.isSelected ? root._bgSelected : (_mouseArea.containsMouse ? root._bgHovered : "transparent")
        border.width: 0

    UTFocusItem {
        delegateFocused: root.isCurrent
        focusRadius: bg.radius
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 0

        // Icon + Name column (flex: fills remaining)
        RowLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.preferredWidth: 300
            spacing: 6

            Loader {
                Layout.minimumWidth: 18
                Layout.preferredWidth: 18
                Layout.maximumWidth: 18
                Layout.preferredHeight: 18
                sourceComponent: root.iconComponent ?? _defaultIcon

                property url fileUrl: model.fileUrl
                property string fileName: model.fileName
            }

            UTText {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                text: model.fileName
                fontEnum: UIFontToken.Body_Text
                color: root.isSelected ? root._textSelected : root._textNormal
                elide: Text.ElideLeft
            }
        }

        // Date column
        UTText {
            visible: !root.compact
            Layout.minimumWidth: 160
            Layout.preferredWidth: 160
            Layout.maximumWidth: 160
            text: Qt.formatDateTime(model.fileModified, "yyyy-MM-dd hh:mm:ss")
            fontEnum: UIFontToken.Caption_Text
            color: root.isSelected ? root._textNormal : root._secondaryText
            leftPadding: 8
            elide: Text.ElideRight
        }

        // Size column
        UTText {
            Layout.minimumWidth: 80
            Layout.preferredWidth: 80
            Layout.maximumWidth: 80
            text: {
                var s = model.fileSize
                if (s === undefined || s === 0) return ""
                if (s > 1048576) return (s / 1048576).toFixed(1) + " MB"
                if (s > 1024) return (s / 1024).toFixed(0) + " KB"
                return s + " B"
            }
            fontEnum: UIFontToken.Caption_Text
            color: root._secondaryText
            horizontalAlignment: Text.AlignRight
            leftPadding: 8
            elide: Text.ElideRight
        }

        // Type column
        UTText {
            visible: !root.compact
            Layout.minimumWidth: 70
            Layout.preferredWidth: 70
            Layout.maximumWidth: 70
            text: {
                var ext = model.fileName.split('.').pop()
                return ext ? ext.toUpperCase() : ""
            }
            fontEnum: UIFontToken.Caption_Text
            color: root._secondaryText
            leftPadding: 8
            elide: Text.ElideRight
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

    } // end bg Rectangle

    // Default icon
    Component {
        id: _defaultIcon
        Text {
            text: "📄"
            // emoji glyph size, intentionally not tokenized
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
