import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0
import "../common"

GalleryCase {
    objectName: "utFocusItemCase"

    title: "UTFocusItem"

    component FocusHost: Rectangle {
        id: focusHost

        property alias text: caption.text

        activeFocusOnTab: true
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Listview_Item_Background, UIColorState.Normal)
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border, UIColorState.Normal)

        UTLabel {
            id: caption
            anchors.centerIn: parent
            width: Math.max(0, parent.width - 12)
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            onPressed: focusHost.forceActiveFocus(Qt.MouseFocusReason)
        }

        UTFocusItem {
            objectName: focusHost.objectName + "Outline"
            target: focusHost
            focusRadius: focusHost.radius
        }
    }

    GalleryDemo {
        title: qsTr("Outline shapes")

        Flow {
            id: focusSamples
            Layout.fillWidth: true
            spacing: 12

            Item {
                width: Math.min(144, focusSamples.width)
                height: 104

                FocusHost {
                    objectName: "squareFocusHost"
                    anchors.centerIn: parent
                    width: Math.min(92, Math.max(0, parent.width - 12))
                    height: width
                    text: qsTr("Square")
                }
            }

            Item {
                width: Math.min(144, focusSamples.width)
                height: 104

                FocusHost {
                    objectName: "roundedFocusHost"
                    anchors.centerIn: parent
                    width: Math.min(132, Math.max(0, parent.width - 12))
                    height: 72
                    radius: 12
                    text: qsTr("Rounded")
                }
            }

            Item {
                width: Math.min(144, focusSamples.width)
                height: 104

                FocusHost {
                    objectName: "circularFocusHost"
                    anchors.centerIn: parent
                    width: Math.min(92, Math.max(0, parent.width - 12))
                    height: width
                    radius: width / 2
                    text: qsTr("Circle")
                }
            }
        }
    }
}
