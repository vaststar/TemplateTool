import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0
import "../common"

GalleryCase {
    objectName: "utImageButtonCase"

    title: "UTImageButton"

    GalleryDemo {
        title: qsTr("32 / 48 px")

        Flow {
            Layout.fillWidth: true
            spacing: 12

            UTImageButton {
                objectName: "imageButton32"
                width: 32
                height: 32
                source: UTComponentUtil.getImageResourcePath(UIAssetImageToken.Icon_User)
                Accessible.name: qsTr("User image button, 32 px")
            }
            UTImageButton {
                objectName: "imageButton48"
                width: 48
                height: 48
                source: UTComponentUtil.getImageResourcePath(UIAssetImageToken.Icon_User)
                Accessible.name: qsTr("User image button, 48 px")
            }
        }
    }

    GalleryDemo {
        title: qsTr("Disabled")

        UTImageButton {
            objectName: "disabledImageButton"
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            enabled: false
            source: UTComponentUtil.getImageResourcePath(UIAssetImageToken.Icon_User)
            Accessible.name: qsTr("Disabled user image button")
        }
    }
}
