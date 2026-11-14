import QtQuick
import QtQuick.Controls.Basic

SpinBox {
    id: control

    property real fieldHeight: 32
    property real buttonWidth: 28
    property int borderRadius: 4

    property color backgroundColor: palette.base
    property color textColor: palette.text
    property color upButtonColor: palette.button
    property color downButtonColor: palette.button
    property color borderColor: palette.mid
    property color selectionColor: palette.highlight
    property color selectedTextColor: palette.highlightedText

    editable: true
    live: true
    implicitWidth: 120
    implicitHeight: fieldHeight

    contentItem: TextInput {
        z: 2
        text: control.textFromValue(control.value, control.locale)
        anchors.fill: parent
        anchors.leftMargin: control.buttonWidth + 4
        anchors.rightMargin: control.buttonWidth + 4
        font: control.font
        color: control.textColor
        selectionColor: control.selectionColor
        selectedTextColor: control.selectedTextColor
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
    }

    up.indicator: Rectangle {
        x: parent.width - width
        height: parent.height
        width: control.buttonWidth
        radius: control.borderRadius
        color: control.upButtonColor
        Behavior on color { ColorAnimation { duration: 100 } }

        Text {
            anchors.centerIn: parent
            text: "+"
            // glyph size, intentionally not tokenized
            font.pixelSize: 14
            font.bold: true
            color: control.textColor
        }
    }

    down.indicator: Rectangle {
        x: 0
        height: parent.height
        width: control.buttonWidth
        radius: control.borderRadius
        color: control.downButtonColor
        Behavior on color { ColorAnimation { duration: 100 } }

        Text {
            anchors.centerIn: parent
            text: "\u2212"
            // glyph size, intentionally not tokenized
            font.pixelSize: 14
            font.bold: true
            color: control.textColor
        }
    }

    background: Rectangle {
        radius: control.borderRadius
        color: control.backgroundColor
    }

    // Border drawn ABOVE the +/- buttons. The background's own border would be
    // occluded on the left/right by the opaque button rectangles, leaving only
    // the centre segment visible around the edit — which reads as a second,
    // inner focus frame. Painting the border here keeps it a single clean
    // rounded rect around the whole control in every state.
    Rectangle {
        z: 5
        anchors.fill: parent
        enabled: false
        radius: control.borderRadius
        color: "transparent"
        border.width: 1
        border.color: control.borderColor
        Behavior on border.color { ColorAnimation { duration: 100 } }
    }
}
