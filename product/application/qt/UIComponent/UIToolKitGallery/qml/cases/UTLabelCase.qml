import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0
import "../common"

GalleryCase {
    objectName: "utLabelCase"

    title: "UTLabel"

    GalleryDemo {
        title: qsTr("Primary / Secondary")

        UTLabel {
            objectName: "primaryLabel"
            Layout.fillWidth: true
            text: qsTr("Body text sample")
            colorEnum: UIColorToken.Content_Text
            wrapMode: Text.Wrap
        }
        UTLabel {
            objectName: "secondaryLabel"
            Layout.fillWidth: true
            text: qsTr("Secondary text sample")
            colorEnum: UIColorToken.Content_Secondary_Text
            wrapMode: Text.Wrap
        }
    }
}
