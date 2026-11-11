import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utGroupBoxCase"

    title: "UTGroupBox"

    GalleryDemo {
        title: qsTr("Grouped controls")

        UTGroupBox {
            objectName: "settingsGroup"
            Layout.fillWidth: true
            Layout.maximumWidth: 320
            title: qsTr("Settings")

            contentItem: ColumnLayout {
                spacing: 10

                UTCheckBox {
                    objectName: "featureCheckBox"
                    text: qsTr("Enable feature")
                    checked: true
                }
                UTSwitch {
                    objectName: "settingsSwitch"
                    checked: false
                }
            }
        }
    }
}
