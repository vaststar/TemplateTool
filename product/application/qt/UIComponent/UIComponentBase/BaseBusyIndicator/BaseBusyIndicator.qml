import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

BusyIndicator {
    id: control

    property int indicatorSize: 36
    property int ringThickness: 3
    property int dotSize: 8
    property int cycleDuration: 900
    property color trackColor: palette.mid
    property color progressColor: palette.highlight

    implicitWidth: indicatorSize
    implicitHeight: indicatorSize

    contentItem: Item {
        implicitWidth: control.indicatorSize
        implicitHeight: control.indicatorSize

        Rectangle {
            anchors.fill: parent
            color: "transparent"
            border.width: control.ringThickness
            border.color: control.trackColor
            radius: width / 2
        }

        Item {
            anchors.fill: parent

            Rectangle {
                width: control.dotSize
                height: control.dotSize
                radius: width / 2
                anchors.horizontalCenter: parent.horizontalCenter
                y: -height / 2 + control.ringThickness / 2
                color: control.progressColor

                Behavior on color { ColorAnimation { duration: 100 } }
            }

            RotationAnimator on rotation {
                from: 0; to: 360
                duration: control.cycleDuration
                loops: Animation.Infinite
                running: control.running
            }
        }
    }
}
