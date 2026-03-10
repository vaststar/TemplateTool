import QtQuick
import QtQuick.Controls

ComboBox {
    id: control

    // === State for subclass ===
    property bool popupIsOpen: false
    property int focusedItemIndex: -1

    // === Configurable ===
    property int borderRadius: 6
    property int itemHeight: 32

    focusPolicy: Qt.StrongFocus

    // === Keyboard navigation with wrap-around ===
    Keys.onSpacePressed: function(event) {
        event.accepted = true
        if (!popupIsOpen) {
            popupIsOpen = true
            popup.open()
        } else {
            selectAndClose()
        }
    }
    Keys.onReturnPressed: function(event) {
        if (popupIsOpen) {
            event.accepted = true
            selectAndClose()
        }
    }
    Keys.onUpPressed: function(event) {
        event.accepted = true
        if (popupIsOpen) {
            // Wrap to bottom when at top
            focusedItemIndex = focusedItemIndex <= 0 ? count - 1 : focusedItemIndex - 1
            highlightedIndex = focusedItemIndex
        } else {
            var newIndex = currentIndex <= 0 ? count - 1 : currentIndex - 1
            currentIndex = newIndex
            activated(newIndex)
        }
    }
    Keys.onDownPressed: function(event) {
        event.accepted = true
        if (popupIsOpen) {
            // Wrap to top when at bottom
            focusedItemIndex = focusedItemIndex >= count - 1 ? 0 : focusedItemIndex + 1
            highlightedIndex = focusedItemIndex
        } else {
            var newIndex = currentIndex >= count - 1 ? 0 : currentIndex + 1
            currentIndex = newIndex
            activated(newIndex)
        }
    }
    Keys.onEscapePressed: function(event) {
        if (popupIsOpen) {
            event.accepted = true
            popup.close()
        }
    }

    function selectAndClose() {
        if (focusedItemIndex >= 0) {
            currentIndex = focusedItemIndex
            activated(focusedItemIndex)
        }
        popup.close()
    }

    // === Popup state sync ===
    popup.onOpened: {
        popupIsOpen = true
        focusedItemIndex = currentIndex
        highlightedIndex = currentIndex
    }
    popup.onClosed: {
        popupIsOpen = false
        focusedItemIndex = -1
    }
}
