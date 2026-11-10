import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utSwitchCase"

    title: "UTSwitch"

    GalleryDemo {
        title: qsTr("On / Off / Disabled")

        Flow {
            Layout.fillWidth: true
            spacing: 12

            UTSwitch {
                objectName: "checkedSwitch"
                checked: true
            }
            UTSwitch {
                objectName: "uncheckedSwitch"
                checked: false
            }
            UTSwitch {
                objectName: "disabledSwitch"
                checked: true
                enabled: false
            }
        }
    }
}
