import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    id: root
    objectName: "utPopupCase"

    title: "UTPopup"

    onVisibleChanged: {
        if (!visible)
            popup.close()
    }
    Component.onDestruction: popup.close()

    GalleryDemo {
        title: qsTr("Anchored popup")

        UTButton {
            id: openButton
            objectName: "openPopupButton"
            text: qsTr("Open popup")
            onClicked: popup.open()
        }
    }

    UTPopup {
        id: popup
        objectName: "demoPopup"
        parent: openButton
        x: 0
        y: openButton.height + 8
        width: Math.min(280, root.width)
        padding: 16
        margins: 8
        focus: true

        contentItem: ColumnLayout {
            spacing: 12

            UTText {
                objectName: "popupContentText"
                Layout.fillWidth: true
                text: qsTr("Popup content")
                wrapMode: Text.Wrap
            }
            UTButton {
                objectName: "closePopupButton"
                text: qsTr("Close")
                focus: true
                onClicked: popup.close()
            }
        }
    }
}
