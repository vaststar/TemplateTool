import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utProgressBarCase"

    title: "UTProgressBar"

    GalleryDemo {
        title: qsTr("Determinate")

        UTProgressBar {
            objectName: "progressBar"
            Layout.fillWidth: true
            Layout.maximumWidth: 320
            value: 0.65
        }
    }
}
