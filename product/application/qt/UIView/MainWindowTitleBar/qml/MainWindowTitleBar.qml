import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
// import QtGraphicalEffects  // 提供模糊
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0
// import "."  // 导入当前目录的所有 QML 组件

Rectangle {
        property alias controller: titleBarController
        MainWindowTitleBarController{
            id: titleBarController
        }
        height: 40
        
        color: controller.color//"#3c5bcaff"  // 设置标题栏颜色
        visible: controller.visible
        required property ApplicationWindow appWindow

        
    //     WindowAnimator {
    //     id: windowAnimator
    //     appWindow: appWindow
    //     parentWindow: appWindow.contentItem
    // }

        RowLayout {
            anchors.fill: parent
            spacing: 10
        UTImageButton {
    // source: "qrc:/qt/qml/UIView/picture/profile.png"
    // sourceHovered: "qrc:/qt/qml/UIView/picture/profile.png"
    // sourcePressed: "qrc:/qt/qml/UIView/picture/profile.png"
    source: "qrc:/images/profile.png"
    sourceHovered: "qrc:/images/profile.png"
    sourcePressed: "qrc:/images/profile.png"
            Layout.preferredWidth: parent.height - 10
            Layout.fillHeight: true
            Layout.margins: 5
    onClicked: {
        console.log("Image button clicked!")
    }
}

        // UTButton {
        //     text: "1"
        //     onClicked: console.log("功能1")
        //     Layout.preferredWidth: parent.height
        //     Layout.fillHeight: true
        //     padding: 0
        // }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.ArrowCursor
                onPressed: (mouse) => {
                    if (mouse.buttons === Qt.LeftButton && appWindow)
                    {
                        appWindow.startSystemMove()
                    }
                }
            }
        }

        Button {
            text: "—"
            enabled: appWindow !== null
            onClicked: appWindow && appWindow.showMinimized()
            Layout.preferredWidth: 40
            Layout.fillHeight: true
            padding: 0
        }


        Button {
            text: appWindow && appWindow.visibility === ApplicationWindow.Maximized ? "🗗" : "🗖"
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
        }

        Button {
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
        }
        }

}