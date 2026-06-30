import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import UIComponentBase 1.0
import UIResourceLoader 1.0
import UTComponent 1.0

BaseComboBox {
    id: control

    // Model role holding the icon source (URL / qrc path / data: base64).
    // Empty => text-only (unchanged legacy behaviour).
    property string iconRole: ""
    property int iconSize: 20
    property var fontEnum: UIFontToken.Combobox_Text

    // Icon source for the currently selected item (collapsed display).
    // contentItem is not in a delegate context, so resolve it from the model.
    readonly property string currentIconSource: {
        if (iconRole === "" || currentIndex < 0)
            return ""
        const m = control.model
        if (m && typeof m.get === "function")        // ListModel
            return m.get(currentIndex)[iconRole] ?? ""
        if (Array.isArray(m))                         // JS array of objects
            return (m[currentIndex] && m[currentIndex][iconRole]) ?? ""
        return ""
    }

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
    contentItem: RowLayout {
        spacing: 8
        Image {
            Layout.leftMargin: 12
            Layout.alignment: Qt.AlignVCenter
            visible: control.iconRole !== "" && status === Image.Ready
            source: control.currentIconSource
            sourceSize: Qt.size(control.iconSize, control.iconSize)
            fillMode: Image.PreserveAspectFit
        }
        UTText {
            id: contentText
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            leftPadding: control.iconRole !== "" ? 0 : 12
            rightPadding: control.indicator.width + control.spacing
            text: control.displayText
            fontEnum: control.fontEnum
            color: control.textColor
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight

            HoverHandler {
                id: contentHoverHandler
            }

            UTToolTip {
                parent: contentText
                text: control.displayText
                visible: contentText.truncated && contentHoverHandler.hovered
                delay: 500
                cursorX: contentHoverHandler.point.position.x
                cursorY: contentHoverHandler.point.position.y
            }
        }
    }

    // === Dropdown indicator ===
    indicator: UTText {
        x: control.width - width - 12
        y: (control.height - height) / 2
        text: control.popup.visible ? "\u25B4" : "\u25BE"
        fontEnum: control.fontEnum
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

    popup.contentItem: Column {
        spacing: 2
        Repeater {
            model: control.popup.visible ? control.delegateModel : null
        }
        Item { width: 1; height: 3 }
    }

    // === Delegate with focus ring ===
    delegate: ItemDelegate {
        id: delegateItem
        required property int index
        required property var model

        width: control.width - 8
        height: control.itemHeight
        padding: 0

        readonly property bool isHighlighted: control.focusedItemIndex === index
        highlighted: isHighlighted

        contentItem: RowLayout {
            spacing: 8
            Image {
                Layout.leftMargin: 12
                Layout.alignment: Qt.AlignVCenter
                visible: control.iconRole !== "" && status === Image.Ready
                source: control.iconRole !== "" ? (delegateItem.model[control.iconRole] ?? "") : ""
                sourceSize: Qt.size(control.iconSize, control.iconSize)
                fillMode: Image.PreserveAspectFit
            }
            UTText {
                id: delegateText
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                leftPadding: control.iconRole !== "" ? 0 : 12
                text: control.textAt(delegateItem.index)
                fontEnum: UIFontToken.Combobox_Text
                color: control.textColor
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight

                HoverHandler {
                    id: delegateHoverHandler
                }

                UTToolTip {
                    parent: delegateText
                    text: delegateText.text
                    visible: delegateText.truncated && delegateHoverHandler.hovered
                    delay: 500
                    cursorX: delegateHoverHandler.point.position.x
                    cursorY: delegateHoverHandler.point.position.y
                }
            }
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
