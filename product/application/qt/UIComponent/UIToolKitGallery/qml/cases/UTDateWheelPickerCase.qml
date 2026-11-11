import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utDateWheelPickerCase"

    title: "UTDateWheelPicker"

    GalleryDemo {
        title: qsTr("Wheel")

        UTDateWheelPicker {
            objectName: "dateWheelPicker"
            Layout.alignment: Qt.AlignLeft
        }
    }
}
