import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utContentDividerCase"

    title: "UTContentDivider"

    GalleryDemo {
        title: qsTr("Vertical")

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            UTText {
                Layout.fillWidth: true
                text: qsTr("Left content")
                wrapMode: Text.Wrap
            }
            UTContentDivider {
                objectName: "contentDivider"
                Layout.preferredHeight: 64
                Layout.alignment: Qt.AlignVCenter
            }
            UTText {
                Layout.fillWidth: true
                text: qsTr("Right content")
                wrapMode: Text.Wrap
            }
        }
    }
}
