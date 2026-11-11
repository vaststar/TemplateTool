import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utWheelColumnCase"

    title: "UTWheelColumn"

    GalleryDemo {
        title: qsTr("Hours · 00–23 · Wrap")

        Item {
            Layout.fillWidth: true
            implicitHeight: 192

            UTWheelColumn {
                id: hoursWheel
                objectName: "hoursWheelColumn"
                x: 6
                y: 6
                width: Math.min(120, Math.max(0, parent.width - 12))
                height: 180
                valueCount: 24
                value: 23
                wrap: true
                onValuePicked: function(v) { value = v }
            }
        }

        UTLabel {
            objectName: "hoursWheelValue"
            Layout.fillWidth: true
            text: qsTr("Value: %1").arg(hoursWheel.formatValue(hoursWheel.value))
            wrapMode: Text.Wrap
        }
    }

    GalleryDemo {
        title: qsTr("Range · 00–10 · No wrap")

        Item {
            Layout.fillWidth: true
            implicitHeight: 192

            UTWheelColumn {
                id: boundedWheel
                objectName: "boundedWheelColumn"
                x: 6
                y: 6
                width: Math.min(120, Math.max(0, parent.width - 12))
                height: 180
                valueCount: 11
                value: 0
                wrap: false
                onValuePicked: function(v) { value = v }
            }
        }

        UTLabel {
            objectName: "boundedWheelValue"
            Layout.fillWidth: true
            text: qsTr("Value: %1").arg(boundedWheel.formatValue(boundedWheel.value))
            wrapMode: Text.Wrap
        }
    }

    GalleryDemo {
        title: qsTr("Available 00–07 · Disabled 08–11")

        Item {
            Layout.fillWidth: true
            implicitHeight: 192

            UTWheelColumn {
                id: disabledTailWheel
                objectName: "disabledTailWheelColumn"
                x: 6
                y: 6
                width: Math.min(120, Math.max(0, parent.width - 12))
                height: 180
                valueCount: 12
                enabledCount: 8
                value: 7
                wrap: false
                onValuePicked: function(v) { value = v }
            }
        }

        UTLabel {
            objectName: "disabledTailWheelValue"
            Layout.fillWidth: true
            text: qsTr("Value: %1").arg(disabledTailWheel.formatValue(disabledTailWheel.value))
            wrapMode: Text.Wrap
        }
    }
}
