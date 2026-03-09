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
        currentIndex: controller.pageIdToIndex(navigationBar.controller.currentPageId)

        HomePage { id: homePage }                             // index 0
        ContactsPage { id: contactsPage }                    // index 1
        PlaceholderPage { pageTitle: "计划" }                 // index 2
        PlaceholderPage { pageTitle: "证件" }                 // index 3
        PlaceholderPage { pageTitle: "工具" }                 // index 4
        SettingsPage { id: settingsPage }                     // index 5
        PlaceholderPage { pageTitle: "帮助" }                 // index 6
        PlaceholderPage { pageTitle: "关于" }                 // index 7
    }

    Component.onCompleted: {
        if (!controller) {
            console.log("MainWindowContent controller is null")
            return
        }
        controller.setupController(navigationBar.controller)
        controller.setupController(homePage.controller)
        controller.setupController(contactsPage.controller)
        controller.setupController(settingsPage.controller)
    }
}