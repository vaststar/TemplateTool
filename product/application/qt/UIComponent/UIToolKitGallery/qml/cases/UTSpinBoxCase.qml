import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utSpinBoxCase"

    title: "UTSpinBox"

    GalleryDemo {
        title: qsTr("Editable / Disabled")

        Flow {
            Layout.fillWidth: true
            spacing: 12

            UTSpinBox {
                objectName: "editableSpinBox"
                from: 0; to: 100; value: 4
            }
            UTSpinBox {
                objectName: "disabledSpinBox"
                from: 0; to: 100; value: 4
                enabled: false
            }
        }
    }
}
