import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utTextFieldCase"

    title: "UTTextField"

    GalleryDemo {
        title: qsTr("Single line")

        UTTextField {
            objectName: "singleLineField"
            Layout.fillWidth: true
            Layout.maximumWidth: 320
            placeholderText: qsTr("Single line...")
        }
    }
}
