import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utTimePickerCase"

    title: "UTTimePicker"

    GalleryDemo {
        title: qsTr("Hours / Minutes")

        UTTimePicker {
            objectName: "timePicker"
            Layout.alignment: Qt.AlignLeft
            hours: 9; minutes: 30
        }
    }
}
