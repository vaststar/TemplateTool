import QtQuick
import UIView 1.0

Item {
    id: contactsPage
    property ContactsPageController controller: ContactsPageController {}

    // Left panel: contact list
    ContactListPanel {
        id: contactListPanel
        controller: contactsPage.controller
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: 200
    }

    // Right panel: contact details (placeholder)
    ContactDetailPanel {
        id: contactDetailPanel
        controller: contactsPage.controller
        anchors {
            left: contactListPanel.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
    }
}
