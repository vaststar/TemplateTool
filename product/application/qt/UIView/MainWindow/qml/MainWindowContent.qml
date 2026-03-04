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
    // PageId: Home=1, Contacts=2, Tasks=3, Credentials=4, Toolbox=5, Settings=6, Help=7, About=8
    StackLayout {
        id: contentStack
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: navigationBar.right
            right: parent.right
        }
        currentIndex: navigationBar.controller.currentPageId - 1

        HomePage { id: homePage }                             // index 0 → Home(1)
        ContactsPage { id: contactsPage }                    // index 1 → Contacts(2)
        PlaceholderPage { pageTitle: "计划" }                 // index 2 → Tasks(3)
        PlaceholderPage { pageTitle: "证件" }                 // index 3 → Credentials(4)
        PlaceholderPage { pageTitle: "工具" }                 // index 4 → Toolbox(5)
        SettingsPage { id: settingsPage }                     // index 5 → Settings(6)
        PlaceholderPage { pageTitle: "帮助" }                 // index 6 → Help(7)
        PlaceholderPage { pageTitle: "关于" }                 // index 7 → About(8)
    }

    Component.onCompleted: {
        if (!controller) {
            console.log("MainWindowContent controller is null")
            return
        }
        controller.initController(navigationBar.controller)
        controller.initController(homePage.controller)
        controller.initController(contactsPage.controller)
        controller.initController(settingsPage.controller)
    }
}