import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utToolButtonCase"

    title: "UTToolButton"

    GalleryDemo {
        title: qsTr("Normal / Disabled")

        Flow {
            Layout.fillWidth: true
            spacing: 12

            UTToolButton {
                objectName: "toolButton"
                text: qsTr("Tool")
                toolTipText: qsTr("Tool button tooltip")
            }
            UTToolButton {
                objectName: "disabledButton"
                text: qsTr("Disabled")
                enabled: false
            }
        }
    }
}
