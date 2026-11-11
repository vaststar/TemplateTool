import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utTextAreaCase"

    title: "UTTextArea"

    GalleryDemo {
        title: qsTr("Multi-line")

        UTTextArea {
            objectName: "multiLineField"
            Layout.fillWidth: true
            Layout.maximumWidth: 320
            Layout.preferredHeight: 90
            placeholderText: qsTr("Multi-line text...")
        }
    }
}
