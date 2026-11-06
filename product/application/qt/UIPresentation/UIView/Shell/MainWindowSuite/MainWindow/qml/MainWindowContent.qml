import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: mainWindowContent
    required property MainWindowController controller

    readonly property real contentInset: 8
    readonly property real contentRadius: 14

    // The shell stays visible around the shared page surface.
    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)
    }

    // Sidebar
    MainWindowSideBar {
        id: navigationBar
        width: implicitWidth
        height: parent.height
        z: 10
    }

    Item {
        id: contentSurface
        objectName: "contentSurface"
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: navigationBar.right
            right: parent.right
            margins: mainWindowContent.contentInset
        }
        clip: true
        layer.enabled: true
        layer.effect: MultiEffect {
            autoPaddingEnabled: false
            maskEnabled: true
            maskSource: contentMask
        }

        Rectangle {
            anchors.fill: parent
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Background, UIColorState.Normal)
        }

        // Page content — StackLayout driven by sidebar currentPageId.
        StackLayout {
            id: contentStack
            anchors.fill: parent
            currentIndex: controller.initialized ? controller.pageRegistry.indexOfPage(navigationBar.controller.currentPageId) : -1

            Repeater {
                model: controller.initialized ? controller.pageRegistry.entries : []
                Loader {
                    id: pageLoader
                    required property var modelData   // { source: string, preload: bool }
                    required property int index

                    // Keep visited pages loaded so their controllers and state survive navigation.
                    property bool everShown: modelData.preload || index === contentStack.currentIndex
                    Connections {
                        target: contentStack
                        function onCurrentIndexChanged() {
                            if (index === contentStack.currentIndex)
                                pageLoader.everShown = true
                        }
                    }

                    active: everShown
                    source: active ? modelData.source : ""

                    onLoaded: {
                        if (item && item.controller)
                            mainWindowContent.controller.setupController(item.controller)
                    }
                }
            }
        }
    }

    // Keep the mask outside the sampled page subtree to avoid a rendering cycle.
    Rectangle {
        id: contentMask
        width: contentSurface.width
        height: contentSurface.height
        radius: mainWindowContent.contentRadius
        color: "white"
        antialiasing: true
        visible: false
        layer.enabled: true
    }

    Component.onCompleted: {
        if (!controller) {
            console.log("MainWindowContent controller is null")
            return
        }
        controller.setupController(navigationBar.controller)
    }
}
