import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0
import "../common"

GalleryCase {
    objectName: "utMenuCase"

    title: "UTMenu"

    GalleryDemo {
        objectName: "menuDemo"
        title: qsTr("Items / Disabled / Separator / Checkable")

        UTButton {
            id: menuButton
            objectName: "menuButton"
            text: qsTr("Open menu")
            onClicked: previewMenu.open()

            UTMenu {
                id: previewMenu
                objectName: "previewMenu"
                x: 0
                y: menuButton.height + 4
                focus: true

                UTMenuItem {
                    objectName: "bodyTextMenuItem"
                    text: qsTr("Body text")
                    onTriggered: menuPreview.fontEnum = UIFontToken.Body_Text
                }
                UTMenuItem {
                    objectName: "headingTextMenuItem"
                    text: qsTr("Heading text")
                    onTriggered: menuPreview.fontEnum = UIFontToken.Heading_Text
                }
                UTMenuItem {
                    objectName: "exportPreviewMenuItem"
                    text: qsTr("Export preview")
                    enabled: false
                }
                UTMenuSeparator {
                    objectName: "previewMenuSeparator"
                }
                UTMenuItem {
                    id: showDetailsItem
                    objectName: "showDetailsMenuItem"
                    text: qsTr("Show details")
                    checkable: true
                    checked: true
                }
            }
        }

        UTLabel {
            id: menuPreview
            objectName: "menuPreview"
            Layout.fillWidth: true
            text: qsTr("A clear view of every detail.")
            fontEnum: UIFontToken.Body_Text
            wrapMode: Text.Wrap
        }
        UTLabel {
            objectName: "menuDetails"
            Layout.fillWidth: true
            visible: showDetailsItem.checked
            text: qsTr("Letters, numbers, punctuation: Aa Bb Cc 0123 !?")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Secondary_Text
            wrapMode: Text.Wrap
        }
    }
}
