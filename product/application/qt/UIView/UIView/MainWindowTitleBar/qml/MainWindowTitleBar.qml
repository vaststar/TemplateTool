import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0

Rectangle {
        height: 40
        color: "#3A3F51"  // 设置标题栏颜色
        property alias controller: controller
        MainWindowTitleBarController {
            id: controller
        }

        RowLayout {
            anchors.fill: parent
            spacing: 10

            Text {
                text: qsTr(controller.title)
                color: "white"
                verticalAlignment: Text.AlignVCenter
                Layout.fillWidth: true
                padding: 10
            }

            Button {
                text: "最小化"
                onClicked: showMinimized()
            }

            Button {
                text: "最大化"
                onClicked: {
                    if (visibility === ApplicationWindow.Maximized) {
                        visibility = ApplicationWindow.Windowed
                    } else {
                        visibility = ApplicationWindow.Maximized
                    }
                }
            }

            Button {
                text: "关闭"
                onClicked: Qt.quit()
            }
        }
    }