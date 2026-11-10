import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utCheckBoxCase"

    title: "UTCheckBox"

    GalleryDemo {
        title: qsTr("Checked / Unchecked / Disabled")

        Flow {
            Layout.fillWidth: true
            spacing: 12

            UTCheckBox {
                objectName: "checkedCheckBox"
                text: qsTr("Check me")
                checked: true
            }
            UTCheckBox {
                objectName: "uncheckedCheckBox"
                text: qsTr("Unchecked")
                checked: false
            }
            UTCheckBox {
                objectName: "disabledCheckBox"
                text: qsTr("Disabled")
                checked: true
                enabled: false
            }
        }
    }
}
