import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utComboBoxCase"

    title: "UTComboBox"

    GalleryDemo {
        title: qsTr("Text")
        UTComboBox {
            objectName: "textComboBox"
            Layout.fillWidth: true
            Layout.maximumWidth: 320
            model: [qsTr("Option A"), qsTr("Option B"), qsTr("Option C")]
        }
    }
    GalleryDemo {
        title: qsTr("Icon and text")
        note: "iconRole (国旗 + 区号)"
        UTComboBox {
            objectName: "iconComboBox"
            Layout.fillWidth: true
            Layout.maximumWidth: 320
            textRole: "name"
            valueRole: "code"
            iconRole: "flag"
            model: ListModel {
                ListElement {
                    name: "China  +86"; code: "+86"
                    flag: "data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' width='60' height='40'><rect width='60' height='40' fill='%23de2910'/><text x='10' y='28' font-size='22' fill='%23ffde00'>★</text></svg>"
                }
                ListElement {
                    name: "Japan  +81"; code: "+81"
                    flag: "data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' width='60' height='40'><rect width='60' height='40' fill='%23ffffff'/><circle cx='30' cy='20' r='12' fill='%23bc002d'/></svg>"
                }
                ListElement {
                    name: "France  +33"; code: "+33"
                    flag: "data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' width='60' height='40'><rect x='0' width='20' height='40' fill='%23002654'/><rect x='20' width='20' height='40' fill='%23ffffff'/><rect x='40' width='20' height='40' fill='%23ce1126'/></svg>"
                }
                ListElement {
                    name: "Germany  +49"; code: "+49"
                    flag: "data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' width='60' height='40'><rect y='0' width='60' height='13.3' fill='%23000000'/><rect y='13.3' width='60' height='13.3' fill='%23dd0000'/><rect y='26.6' width='60' height='13.4' fill='%23ffce00'/></svg>"
                }
                ListElement {
                    name: "Italy  +39"; code: "+39"
                    flag: "data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' width='60' height='40'><rect x='0' width='20' height='40' fill='%23008C45'/><rect x='20' width='20' height='40' fill='%23ffffff'/><rect x='40' width='20' height='40' fill='%23CD212A'/></svg>"
                }
            }
        }
    }
}
