import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utTextCase"

    title: "UTText"

    GalleryDemo {
        title: qsTr("Default / Wrapped")

        UTText {
            objectName: "defaultText"
            Layout.fillWidth: true
            text: qsTr("Body text sample")
        }
        UTText {
            objectName: "wrappedText"
            Layout.fillWidth: true
            text: qsTr("This longer text wraps automatically when the available width becomes narrow.")
            wrapMode: Text.Wrap
        }
    }
}
