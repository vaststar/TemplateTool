import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

ProgressBar {
    id: control

    property int barHeight: 8
    property color trackColor: palette.mid
    property color progressColor: palette.highlight

    implicitWidth: 200
    implicitHeight: barHeight

    from: 0
    to: 1.0

    background: Rectangle {
        implicitHeight: control.barHeight
        color: control.trackColor
        radius: height / 2
    }

    contentItem: Item {
        implicitWidth: 200
        implicitHeight: control.barHeight

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: control.progressColor

            Behavior on color { ColorAnimation { duration: 100 } }
        }
    }
}
