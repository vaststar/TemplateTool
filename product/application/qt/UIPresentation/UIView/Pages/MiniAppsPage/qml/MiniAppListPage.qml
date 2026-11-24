import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
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
                appearance: UTButton.Primary
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

        UTGridView {
            id: appGrid
            viewMargin: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: count > 0
            cellWidth: 140
            cellHeight: 140
            model: miniAppsPage.controller.miniApps

            // Restore the same app after the menu temporarily takes focus.
            onCurrentIndexChanged: if (currentIndex >= 0) selectedIndex = currentIndex

            onItemInvoked: function(index) {
                const app = miniAppsPage.controller.miniApps[index]
                if (app)
                    miniAppsPage.controller.launchMiniApp(app.id)
            }

            Shortcut {
                sequences: ["Shift+F10", "Menu"]
                context: Qt.WindowShortcut
                enabled: appGrid.gridView.activeFocus && appGrid.currentIndex >= 0
                autoRepeat: false
                onActivated: {
                    const app = miniAppsPage.controller.miniApps[appGrid.currentIndex]
                    if (!app)
                        return
                    miniAppsPage.contextAppId = app.id
                    const item = appGrid.gridView.currentItem
                    const pos = item
                              ? item.mapToItem(miniAppsPage, item.width / 2, item.height / 2)
                              : appGrid.gridView.mapToItem(miniAppsPage, 0, 0)
                    contextMenu.popupAt(pos.x, pos.y, true)
                }
            }

            delegate: T.Button {
                id: appCell
                width: appGrid.cellWidth
                height: appGrid.cellHeight
                required property int index
                required property var modelData

                focusPolicy: Qt.NoFocus
                Accessible.name: appCell.modelData.name

                onClicked: {
                    appGrid.focusOnItem(appCell.index)
                    appGrid.itemInvoked(appCell.index)
                }

                Column {
                    anchors.centerIn: parent
                    width: parent.width - 24
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
                            Accessible.ignored: true
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
                            Accessible.ignored: true
                            fontEnum: UIFontToken.Subtitle_Text
                            colorEnum: UIColorToken.Sidebar_Item_Text
                        }
                    }

                    UTText {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        text: appCell.modelData.name
                        Accessible.ignored: true
                        fontEnum: UIFontToken.Subtitle_Text
                        colorEnum: UIColorToken.Sidebar_Item_Text
                    }
                }

                UTFocusItem {
                    delegateFocused: appGrid.gridView.activeFocus
                                     && appGrid.currentIndex === appCell.index
                    focusMargin: 0
                    focusRadius: 16
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        appGrid.focusOnItem(appCell.index)
                        miniAppsPage.contextAppId = appCell.modelData.id
                        contextMenu.popup()
                    }
                }
            }
        }
    }

    // Per-app context menu (mouse or keyboard).
    UTMenu {
        id: contextMenu
        parent: miniAppsPage

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

