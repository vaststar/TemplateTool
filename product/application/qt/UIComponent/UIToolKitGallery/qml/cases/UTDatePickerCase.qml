import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utDatePickerCase"

    title: "UTDatePicker"

    GalleryDemo {
        title: qsTr("Calendar")

        UTDatePicker {
            objectName: "datePicker"
            Layout.alignment: Qt.AlignLeft
        }
    }
}
