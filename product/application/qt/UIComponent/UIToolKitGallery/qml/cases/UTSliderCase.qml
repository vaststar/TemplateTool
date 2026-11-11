import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utSliderCase"

    title: "UTSlider"

    GalleryDemo {
        title: qsTr("Horizontal")

        UTSlider {
            objectName: "slider"
            Layout.fillWidth: true
            Layout.maximumWidth: 320
            value: 0.4
        }
    }
}
