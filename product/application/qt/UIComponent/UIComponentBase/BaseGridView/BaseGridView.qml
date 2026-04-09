import QtQuick
import QtQuick.Controls

/**
 * BaseGridView - Focus-ring-safe wrapper around GridView.
 *
 * Uses the same clip-separation pattern as BaseListView / BaseTreeView:
 * - Outer FocusScope clips at the container boundary
 * - Inner GridView has clip: false with margins, so focus rings
 *   can extend beyond the GridView into the margin space.
 *
 * Provides:
 * - Focus management: auto currentIndex on focus gain/loss
 * - Keyboard: Enter/Return/Space → itemInvoked
 * - contentAvailableWidth: grid width minus scrollbar
 * - focusOnItem(index): focus + currentIndex for delegate use
 *
 * Subclasses override verticalScrollBar to supply a themed scrollbar.
 */
FocusScope {
    id: root

    property alias gridView: gridView
    property alias model: gridView.model
    property alias delegate: gridView.delegate
    property alias header: gridView.header
    property alias footer: gridView.footer
    property alias cellWidth: gridView.cellWidth
    property alias cellHeight: gridView.cellHeight
    property alias currentIndex: gridView.currentIndex
    property alias count: gridView.count
    property var verticalScrollBar: null
    property int selectedIndex: -1
    property ItemSelectionModel selectionModel: ItemSelectionModel {}
    property real viewMargin: 4
    property real topMargin: viewMargin
    property real bottomMargin: viewMargin
    property real leftMargin: viewMargin
    property real rightMargin: viewMargin

    readonly property real contentAvailableWidth: {
        var sb = root.verticalScrollBar
        return gridView.width - (sb && sb.visible ? sb.width : 0)
    }

    signal itemInvoked(int index)

    onModelChanged: {
        if (selectionModel) selectionModel.model = model
    }

    function focusOnItem(index) {
        gridView.forceActiveFocus()
        gridView.currentIndex = index
        if (selectionModel && selectionModel.model) {
            selectionModel.setCurrentIndex(
                selectionModel.model.index(index, 0),
                ItemSelectionModel.NoUpdate)
        }
    }

    clip: true
    activeFocusOnTab: true

    TapHandler {
        onTapped: gridView.forceActiveFocus()
    }

    GridView {
        id: gridView
        anchors.fill: parent
        anchors.topMargin: root.topMargin
        anchors.bottomMargin: root.bottomMargin
        anchors.leftMargin: root.leftMargin
        anchors.rightMargin: root.rightMargin
        clip: false
        activeFocusOnTab: false
        focus: true
        highlight: Item {}
        highlightFollowsCurrentItem: true
        keyNavigationEnabled: true
        interactive: true
        ScrollBar.vertical: root.verticalScrollBar

        onActiveFocusChanged: {
            if (activeFocus && count > 0 && currentIndex < 0)
                currentIndex = root.selectedIndex >= 0 ? root.selectedIndex : 0
            else if (!activeFocus)
                currentIndex = -1
        }

        Keys.onReturnPressed: function(e) { e.accepted = true; if (currentIndex >= 0) root.itemInvoked(currentIndex) }
        Keys.onEnterPressed:  function(e) { e.accepted = true; if (currentIndex >= 0) root.itemInvoked(currentIndex) }
        Keys.onSpacePressed:  function(e) { e.accepted = true; if (currentIndex >= 0) root.itemInvoked(currentIndex) }
    }
}
