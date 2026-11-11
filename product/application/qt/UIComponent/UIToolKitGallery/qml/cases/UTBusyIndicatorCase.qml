import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utBusyIndicatorCase"

    title: "UTBusyIndicator"

    GalleryDemo {
        title: qsTr("Running")

        UTBusyIndicator {
            objectName: "busyIndicator"
            Layout.alignment: Qt.AlignLeft
            running: true
        }
    }
}
