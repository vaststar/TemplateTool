import QtQuick
import QtQuick.Controls.Basic
import UIComponentBase 1.0

CheckBox {
    id: control

    property real boxSize: 18
    property real boxRadius: 3
    property real checkmarkSize: 13

    property font textFont: font
    property color boxColor: palette.base
    property color checkColor: palette.highlightedText
    property color textColor: palette.text
    property color borderColor: palette.mid

    padding: 0
    spacing: 6

    indicator: Rectangle {
        implicitWidth: control.boxSize
        implicitHeight: control.boxSize
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: control.boxRadius
        color: control.boxColor
        border.width: control.checkState === Qt.Unchecked ? 1 : 0
        border.color: control.borderColor

        Behavior on color { ColorAnimation { duration: 150 } }

        Text {
            anchors.centerIn: parent
            text: control.checkState === Qt.PartiallyChecked ? "\u2212" : "\u2713"
            // glyph size, intentionally not tokenized
            font.pixelSize: control.checkmarkSize
            font.bold: true
            color: control.checkColor
            visible: control.checkState !== Qt.Unchecked
        }
    }

    contentItem: BaseText {
        text: control.text
        font: control.textFont
        color: control.textColor
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.text.length > 0 ? control.indicator.width + control.spacing : control.indicator.width
        visible: control.text.length > 0
    }
}
