import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Rectangle {
    id: root
    property MainWindowTitleBarController controller: MainWindowTitleBarController{}
    
    color: UTComponentUtil.getPlainUIColor(UIColorToken.Titlebar_Background, UIColorState.Normal)
    visible: controller.visible
    required property ApplicationWindow appWindow
        
    RowLayout {
        anchors.fill: parent
        spacing: 5
        UTImageButton {
            Layout.preferredWidth: parent.height - 10
            Layout.fillHeight: true
            Layout.margins: 5
            source: UTComponentUtil.getImageResourcePath(UIAssetImageToken.Icon_User)//"qrc:/images/icon_user"
            onClicked: {
                console.log("Image button clicked!")
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            UTText {
                anchors.centerIn: parent
                text: qsTr(controller.title)
                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Titlebar_Button_Text
                maximumLineCount: 1
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.ArrowCursor
                onPressed: (mouse) => {
                    if (mouse.buttons === Qt.LeftButton && appWindow)
                    {
                        appWindow.startSystemMove()
                    }
                }
                onDoubleClicked: (mouse) => {
                    if (mouse.button === Qt.LeftButton && appWindow) {
                        if (appWindow.visibility === ApplicationWindow.Maximized)
                            appWindow.showNormal()
                        else
                            appWindow.showMaximized()
                    }
                }
            }
        }

        RowLayout {
            spacing: 0
            Layout.fillHeight: true

            UTButton {
                id: minimizeBtn
                text: "—"
                enabled: appWindow !== null
                onClicked: appWindow && appWindow.showMinimized()
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                padding: 0
                focusPolicy: Qt.NoFocus
                backgroundColorEnum: UIColorToken.Titlebar_Button_Background
                borderWidth: 0
                radius: 0
                fontColorEnum: UIColorToken.Titlebar_Button_Text
            }

            UTButton {
                id: maximizeBtn
                text: appWindow && appWindow.visibility === ApplicationWindow.Maximized ? "❐" : "□"
                enabled: appWindow !== null
                onClicked: {
                    if (!appWindow) return
                    if (appWindow.visibility === ApplicationWindow.Maximized)
                        appWindow.showNormal()
                    else
                        appWindow.showMaximized()
                }
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                padding: 0
                focusPolicy: Qt.NoFocus
                backgroundColorEnum: UIColorToken.Titlebar_Button_Background
                borderWidth: 0
                radius: 0
                fontColorEnum: UIColorToken.Titlebar_Button_Text
            }

            UTButton {
                id: closeBtn
                text: "✕"
                onClicked: {
                    if (appWindow)
                        appWindow.close()
                    else
                        Qt.quit()
                }
                Layout.preferredWidth: 40
                Layout.fillHeight: true
                padding: 0
                focusPolicy: Qt.NoFocus
                backgroundColorEnum: UIColorToken.Titlebar_Close_Background
                borderWidth: 0
                radius: 0
                fontColorEnum: UIColorToken.Titlebar_Close_Text
            }
        }
    }

}