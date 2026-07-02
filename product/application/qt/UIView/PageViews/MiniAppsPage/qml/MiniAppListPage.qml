import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: miniAppsPage
    property MiniAppListPageController controller: MiniAppListPageController {}

    // App targeted by the context menu.
    property string contextAppId: ""

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        // Toolbar
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            UTText {
                text: qsTr("Mini Apps")
                fontEnum: UIFontToken.Title_Text
                colorEnum: UIColorToken.Content_Heading
            }

            Item { Layout.fillWidth: true }

            UTButton {
                text: qsTr("Add Mini App")
                onClicked: installDialog.open()
            }
        }

        // Empty state
        UTText {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 48
            visible: appGrid.count === 0
            text: qsTr("No mini apps installed. Click \"Add Mini App\" to install one.")
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Content_Secondary_Text
        }

        GridView {
            id: appGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: count > 0
            cellWidth: 140
            cellHeight: 140
            model: miniAppsPage.controller.miniApps

            delegate: Item {
                id: appCell
                width: appGrid.cellWidth
                height: appGrid.cellHeight
                required property var modelData

                Column {
                    anchors.centerIn: parent
                    spacing: 10

                    Rectangle {
                        width: 72
                        height: 72
                        radius: 16
                        anchors.horizontalCenter: parent.horizontalCenter
                        clip: true
                        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Normal)

                        Image {
                            id: appIcon
                            anchors.fill: parent
                            source: appCell.modelData.iconUrl !== undefined ? appCell.modelData.iconUrl : ""
                            fillMode: Image.PreserveAspectCrop
                            visible: source !== "" && status === Image.Ready
                            asynchronous: true
                            cache: true
                        }

                        UTText {
                            anchors.centerIn: parent
                            visible: !appIcon.visible
                            text: appCell.modelData.name.length > 0 ? appCell.modelData.name.charAt(0) : "?"
                            fontEnum: UIFontToken.Subtitle_Text
                            colorEnum: UIColorToken.Sidebar_Item_Text
                        }
                    }

                    UTText {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: appCell.modelData.name
                        fontEnum: UIFontToken.Subtitle_Text
                        colorEnum: UIColorToken.Sidebar_Item_Text
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    cursorShape: Qt.PointingHandCursor
                    onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                            miniAppsPage.contextAppId = appCell.modelData.id
                            contextMenu.popup()
                        } else {
                            miniAppsPage.controller.launchMiniApp(appCell.modelData.id)
                        }
                    }
                }
            }
        }
    }

    // Per-app context menu (right-click).
    UTMenu {
        id: contextMenu

        UTMenuItem {
            text: qsTr("Launch")
            onTriggered: miniAppsPage.controller.launchMiniApp(miniAppsPage.contextAppId)
        }
        UTMenuSeparator {}
        UTMenuItem {
            text: qsTr("Uninstall")
            onTriggered: miniAppsPage.controller.uninstallMiniApp(miniAppsPage.contextAppId)
        }
    }

    // Native folder picker to install an unpacked mini app.
    FolderDialog {
        id: installDialog
        title: qsTr("Select Mini App Folder")
        onAccepted: miniAppsPage.controller.installMiniApp(selectedFolder)
    }
}

