import QtQuick
import UIView 1.0
import UTComponent 1.0

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

    UTContentDivider {
        id: contentDivider
        anchors {
            left: contactListPanel.right
            top: parent.top
            bottom: parent.bottom
        }
        width: implicitWidth
    }

    // Right panel: contact details
    ContactDetailPanel {
        id: contactDetailPanel
        controller: contactsPage.controller
        anchors {
            left: contentDivider.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
    }
}
