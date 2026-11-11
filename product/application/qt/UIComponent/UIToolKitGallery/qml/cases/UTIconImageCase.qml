import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0
import "../common"

GalleryCase {
    objectName: "utIconImageCase"

    title: "UTIconImage"

    GalleryDemo {
        title: qsTr("16 / 24 / 32 px")

        Flow {
            Layout.fillWidth: true
            spacing: 12

            UTIconImage {
                objectName: "iconImage16"
                iconSize: 16
                imageSourceEnum: UIAssetImageToken.Nav_Home
            }
            UTIconImage {
                objectName: "iconImage24"
                imageSourceEnum: UIAssetImageToken.Nav_Home
            }
            UTIconImage {
                objectName: "iconImage32"
                iconSize: 32
                imageSourceEnum: UIAssetImageToken.Nav_Home
            }
        }
    }

    GalleryDemo {
        title: qsTr("Normal / Selected")

        Flow {
            Layout.fillWidth: true
            spacing: 12

            UTIconImage {
                objectName: "normalIconImage"
                imageSourceEnum: UIAssetImageToken.Nav_Home
            }
            UTIconImage {
                objectName: "selectedIconImage"
                imageSourceEnum: UIAssetImageToken.Nav_Home
                colorState: UIColorState.Selected
            }
        }
    }
}
