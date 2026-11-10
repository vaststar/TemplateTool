import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utButtonCase"

    title: "UTButton"

    GalleryDemo {
        title: qsTr("Primary / Disabled")

        Flow {
            Layout.fillWidth: true
            spacing: 12

            UTButton {
                objectName: "primaryButton"
                text: qsTr("Primary")
            }
            UTButton {
                objectName: "disabledButton"
                text: qsTr("Disabled")
                enabled: false
            }
        }
    }
}
