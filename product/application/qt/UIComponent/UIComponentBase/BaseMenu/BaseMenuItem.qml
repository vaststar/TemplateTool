import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

MenuItem {
    id: control

    property int itemHeight: 32
    property int itemPadding: 12
    property int iconSpacing: 8
    property font shortcutFont: font
    property color textColor: palette.windowText
    property color shortcutColor: palette.windowText
    property color backgroundColor: "transparent"
    property real backgroundRadius: 4

    height: itemHeight
    leftPadding: itemPadding
    rightPadding: itemPadding

    contentItem: RowLayout {
        spacing: control.iconSpacing
        LayoutMirroring.enabled: false
        layoutDirection: control.mirrored ? Qt.RightToLeft : Qt.LeftToRight

        Item {
            visible: control.checkable && control.indicator
            implicitWidth: control.indicator ? control.indicator.width : 0
        }

        Text {
            Layout.fillWidth: true
            text: control.text
            font: control.font
            color: control.textColor
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            visible: control.action && control.action.shortcut
            text: control.action ? control.action.shortcut : ""
            font: control.shortcutFont
            color: control.shortcutColor
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        Item {
            visible: control.subMenu && control.arrow
            implicitWidth: control.arrow ? control.arrow.width : 0
        }
    }

    background: Rectangle {
        radius: control.backgroundRadius
        color: control.backgroundColor

        Behavior on color { ColorAnimation { duration: 100 } }
    }
}
