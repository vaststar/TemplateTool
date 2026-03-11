import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UIResourceLoader 1.0
import UTComponent 1.0

BaseComboBox {
    id: control

    // === Theme colors (new tokens) ===
    readonly property color bgColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Background, UIColorState.Normal)
    readonly property color hoveredColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Background, UIColorState.Hovered)
    readonly property color pressedColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Background, UIColorState.Pressed)
    readonly property color textColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Text, UIColorState.Normal)
    readonly property color borderColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Border, UIColorState.Normal)
    readonly property color focusColor: UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)
    readonly property color itemHoveredColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Item_Background, UIColorState.Hovered)
    readonly property color itemSelectedColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Item_Background, UIColorState.Selected)

    // === ComboBox background ===
    background: Rectangle {
        radius: control.borderRadius
        color: control.pressed ? control.pressedColor
             : control.hovered ? control.hoveredColor
             : control.bgColor
        border.width: 1
        border.color: control.borderColor
    }

    // === ComboBox text (using font token) ===
    contentItem: UTText {
        leftPadding: 12
        rightPadding: control.indicator.width + control.spacing
        text: control.displayText
        fontEnum: UIFontToken.Combobox_Text
        color: control.textColor
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    // === Dropdown indicator ===
    indicator: UTText {
        x: control.width - width - 12
        y: (control.height - height) / 2
        text: "\u25BE"
        fontEnum: UIFontToken.Combobox_Text
        color: control.textColor
    }

    // === Focus ring for ComboBox ===
    UTFocusItem {
        target: control.popup.visible ? null : control
        focusRadius: control.borderRadius
    }

    // === Popup styling ===
    popup.padding: 4
    popup.background: Rectangle {
        radius: control.borderRadius
        color: control.bgColor
        border.width: 1
        border.color: control.borderColor
        layer.enabled: true
    }

    popup.contentItem: ListView {
        id: popupListView
        implicitHeight: contentHeight
        model: control.popup.visible ? control.delegateModel : null
        clip: true
        spacing: 2
        boundsBehavior: Flickable.StopAtBounds
        ScrollIndicator.vertical: ScrollIndicator {}

        // Reset scroll position when popup opens
        Connections {
            target: control.popup
            function onOpened() {
                popupListView.positionViewAtBeginning()
            }
        }

        // Scroll to focused item when navigating with keyboard
        Connections {
            target: control
            function onFocusedItemIndexChanged() {
                if (control.popupIsOpen && control.focusedItemIndex >= 0) {
                    popupListView.positionViewAtIndex(control.focusedItemIndex, ListView.Contain)
                }
            }
        }
    }

    // === Delegate with focus ring ===
    delegate: ItemDelegate {
        id: delegateItem
        required property var modelData
        required property int index

        width: control.width - 8
        height: control.itemHeight
        padding: 0

        readonly property bool isHighlighted: control.focusedItemIndex === index
        highlighted: isHighlighted

        contentItem: UTText {
            leftPadding: 12
            text: delegateItem.modelData
            fontEnum: UIFontToken.Combobox_Text
            color: control.textColor
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            radius: control.borderRadius - 2
            color: delegateItem.isHighlighted || delegateItem.hovered
                ? control.itemHoveredColor
                : control.currentIndex === delegateItem.index
                    ? control.itemSelectedColor
                    : "transparent"
        }

        UTFocusItem {
            target: delegateItem.background
            delegateFocused: delegateItem.isHighlighted && control.popupIsOpen
            focusRadius: control.borderRadius - 2
        }
    }
}
