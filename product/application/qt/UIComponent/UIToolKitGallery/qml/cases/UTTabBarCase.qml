import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utTabBarCase"

    title: "UTTabBar / UTTabButton"

    GalleryDemo {
        title: qsTr("Content pages")

        UTTabBar {
            id: contentTabs
            objectName: "contentTabBar"
            Layout.fillWidth: true
            Layout.maximumWidth: 360
            Layout.margins: 4

            UTTabButton {
                objectName: "infoTabButton"
                text: qsTr("Info")
            }
            UTTabButton {
                objectName: "disabledTabButton"
                text: qsTr("Disabled")
                enabled: false
            }
            UTTabButton {
                objectName: "formTabButton"
                text: qsTr("Form")
            }
        }

        StackLayout {
            objectName: "tabContentStack"
            Layout.fillWidth: true
            Layout.maximumWidth: 360
            Layout.leftMargin: 4
            Layout.rightMargin: 4
            currentIndex: contentTabs.currentIndex

            ColumnLayout {
                objectName: "infoTabPage"
                Layout.fillHeight: false
                spacing: 8

                UTLabel {
                    Layout.fillWidth: true
                    text: qsTr("Workspace")
                    wrapMode: Text.Wrap
                }
                UTLabel {
                    objectName: "workspaceValue"
                    Layout.fillWidth: true
                    text: qsTr("Gallery preview")
                    wrapMode: Text.Wrap
                }
            }

            UTLabel {
                objectName: "disabledTabPage"
                Layout.fillHeight: false
                text: qsTr("Unavailable")
                wrapMode: Text.Wrap
            }

            ColumnLayout {
                objectName: "formTabPage"
                Layout.fillHeight: false
                spacing: 8

                UTLabel {
                    Layout.fillWidth: true
                    text: qsTr("Display name")
                    wrapMode: Text.Wrap
                }
                UTTextField {
                    objectName: "tabDisplayNameField"
                    Layout.fillWidth: true
                    text: qsTr("Gallery user")
                }
                UTCheckBox {
                    objectName: "tabHintsCheckBox"
                    text: qsTr("Show hints")
                    checked: true
                }
            }
        }
    }
}
