import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0
import "../common"

GalleryCase {
    objectName: "utDynamicMenuCase"

    title: "UTDynamicMenu"

    GalleryDemo {
        objectName: "dynamicMenuDemo"
        title: qsTr("Nested choices / Separator")

        UTButton {
            id: dynamicMenuButton
            objectName: "dynamicMenuButton"
            text: qsTr("Open menu")
            onClicked: styleMenu.open()

            UTDynamicMenu {
                id: styleMenu
                objectName: "styleMenu"
                x: 0
                y: dynamicMenuButton.height + 4
                focus: true
                model: [
                    { text: qsTr("Font"), children: [
                        { text: qsTr("Body"), action: "font-body" },
                        { text: qsTr("Heading"), action: "font-heading" },
                        { text: qsTr("Caption"), action: "font-caption" }
                    ] },
                    { text: qsTr("Color"), children: [
                        { text: qsTr("Primary"), action: "color-primary" },
                        { text: qsTr("Secondary"), action: "color-secondary" }
                    ] },
                    { separator: true },
                    { text: qsTr("Reset style"), action: "reset" }
                ]
                onItemTriggered: function(action) {
                    switch (action) {
                    case "font-body":
                        dynamicMenuPreview.fontEnum = UIFontToken.Body_Text
                        break
                    case "font-heading":
                        dynamicMenuPreview.fontEnum = UIFontToken.Heading_Text
                        break
                    case "font-caption":
                        dynamicMenuPreview.fontEnum = UIFontToken.Caption_Text
                        break
                    case "color-primary":
                        dynamicMenuPreview.colorEnum = UIColorToken.Content_Text
                        break
                    case "color-secondary":
                        dynamicMenuPreview.colorEnum = UIColorToken.Content_Secondary_Text
                        break
                    case "reset":
                        dynamicMenuPreview.fontEnum = UIFontToken.Body_Text
                        dynamicMenuPreview.colorEnum = UIColorToken.Content_Text
                        break
                    }
                }
            }
        }

        UTLabel {
            id: dynamicMenuPreview
            objectName: "dynamicMenuPreview"
            Layout.fillWidth: true
            text: qsTr("A clear view of every detail.")
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Text
            wrapMode: Text.Wrap
        }
    }
}
