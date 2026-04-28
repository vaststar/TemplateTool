import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: mainWindowContent
    required property MainWindowController controller

    // Content area background
    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)
    }

    // Sidebar
    MainWindowSideBar {
        id: navigationBar
        width: implicitWidth
        height: parent.height
        z: 10
    }

    // Page content — StackLayout driven by sidebar currentPageId
    StackLayout {
        id: contentStack
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: navigationBar.right
            right: parent.right
        }
        currentIndex: controller.initialized ? controller.pageRegistry.indexOfPage(navigationBar.controller.currentPageId) : -1

        Repeater {
            model: controller.initialized ? controller.pageRegistry.entries : []
            Loader {
                required property string modelData
                required property int index
                source: modelData
                onLoaded: {
                    if (item && item.controller)
                        mainWindowContent.controller.setupController(item.controller)
                }
            }
        }
    }

    Component.onCompleted: {
        if (!controller) {
            console.log("MainWindowContent controller is null")
            return
        }
        controller.setupController(navigationBar.controller)
    }
}
