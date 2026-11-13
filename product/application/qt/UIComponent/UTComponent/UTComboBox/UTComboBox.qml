import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import UIComponentBase 1.0
import UIResourceLoader 1.0
import UTComponent 1.0

BaseComboBox {
    id: control

    property var fontEnum: UIFontToken.Combobox_Text
    popupMaximumHeight: 320
    font: UTComponentUtil.getUIFont(control.fontEnum)
    readonly property int _colorState: control.enabled ? UIColorState.Normal : UIColorState.Disabled

    // === Theme colors (new tokens) ===
    readonly property color bgColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Background, UIColorState.Normal)
    readonly property color hoveredColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Background, UIColorState.Hovered)
    readonly property color pressedColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Background, UIColorState.Pressed)
    readonly property color textColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Text, control._colorState)
    readonly property color borderColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Border, control._colorState)
    readonly property color focusColor: UTComponentUtil.getPlainUIColor(UIColorToken.Focus_Outline, UIColorState.Normal)
    readonly property color itemHoveredColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Item_Background, UIColorState.Hovered)
    readonly property color itemSelectedColor: UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Item_Background, UIColorState.Selected)

    backgroundColor: !control.enabled
                   ? UTComponentUtil.getPlainUIColor(UIColorToken.Combobox_Background, UIColorState.Disabled)
                   : control.pressed ? control.pressedColor
                   : control.hovered ? control.hoveredColor
                   : control.bgColor
    backgroundBorderColor: control.borderColor
    indicatorColor: control.textColor
    indicatorFont: UTComponentUtil.getUIFont(control.fontEnum)

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

    popupItemSpacing: 2
    popupVerticalScrollBar: UTScrollBar { showArrows: false }

    // === Delegate with focus ring ===
    delegate: ItemDelegate {
        id: delegateItem
        required property int index
        required property var model

        width: control.popupContentWidth
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
                fontEnum: control.fontEnum
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
