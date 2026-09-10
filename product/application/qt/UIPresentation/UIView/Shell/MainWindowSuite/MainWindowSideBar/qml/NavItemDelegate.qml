import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

ItemDelegate {
    id: navItem
    height: isVisible ? 44 : 0
    visible: isVisible
    enabled: isEnabled
    hoverEnabled: true
    activeFocusOnTab: true
    leftPadding: 9
    rightPadding: 8
    topPadding: 0
    bottomPadding: 0

    required property int index
    required property int pageId
    required property string itemId
    required property string title
    required property var iconToken
    required property var selectedIconToken
    required property int badge
    required property bool isEnabled
    required property bool isVisible
    required property bool hasSubMenu

    property bool isSelected: false
    property bool showText: false
    property int animationDuration: 200

    background: Rectangle {
        radius: 8
        color: {
            if (navItem.down) {
                return UTComponentUtil.getPlainUIColor(
                    UIColorToken.Sidebar_Item_Background, UIColorState.Pressed)
            }
            if (navItem.isSelected) {
                return UTComponentUtil.getPlainUIColor(
                    UIColorToken.Sidebar_Item_Background, UIColorState.Selected)
            }
            if (navItem.hovered) {
                return UTComponentUtil.getPlainUIColor(
                    UIColorToken.Sidebar_Item_Background, UIColorState.Hovered)
            }
            return UTComponentUtil.getPlainUIColor(
                UIColorToken.Sidebar_Item_Background, UIColorState.Normal)
        }
    }

    contentItem: RowLayout {
        spacing: 8

        // Icon (tinted to match sidebar theme)
        UTIconImage {
            id: navIcon
            Layout.preferredWidth: iconSize
            Layout.preferredHeight: iconSize
            imageSourceEnum: navItem.isSelected ? navItem.selectedIconToken : navItem.iconToken
            colorEnum: UIColorToken.Sidebar_Item_Text
            colorState: navItem.isSelected ? UIColorState.Selected : UIColorState.Normal
        }

        // Label (only visible when expanded)
        UTText {
            Layout.fillWidth: true
            text: navItem.title
            fontEnum: UIFontToken.Body_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
            colorState: navItem.isSelected ? UIColorState.Selected : UIColorState.Normal
            font.weight: navItem.isSelected ? Font.Medium : Font.Normal
            visible: opacity > 0
            opacity: navItem.showText ? 1.0 : 0.0

            Behavior on opacity {
                NumberAnimation { duration: navItem.animationDuration; easing.type: Easing.InOutQuad }
            }
        }

    }

    // Badge – collapsed: small dot on icon top-right; expanded: inline pill on the right
    Rectangle {
        id: badgeRect
        visible: navItem.badge > 0
        radius: height / 2
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Badge_Background, UIColorState.Normal)
        z: 10

        // Collapsed: 14×14 circle anchored to icon top-right
        // Expanded: 16-high pill anchored to right, vertically centered
        width: navItem.showText ? Math.max(18, badgeText.implicitWidth + 6) : 14
        height: navItem.showText ? 16 : 14

        // Icon top-left in navItem: x=9, y=(44-24)/2=10
        // Icon bottom-right: x=33, y=34
        // Collapsed badge: top-right corner of icon, offset outward
        x: navItem.showText ? (navItem.width - width - 8) : (9 + 24 - 6)
        y: navItem.showText ? (navItem.height - height) / 2 : (((navItem.height - 24) / 2) - 4)

        UTText {
            id: badgeText
            anchors.centerIn: parent
            text: navItem.badge > 99 ? "99+" : navItem.badge.toString()
            fontEnum: UIFontToken.Badge_Text
            colorEnum: UIColorToken.Sidebar_Badge_Text
        }
    }

    // Disabled overlay
    Rectangle {
        anchors.fill: parent
        radius: 8
        color: Qt.alpha(UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Background, UIColorState.Normal), 0.5)
        visible: !navItem.isEnabled
    }

    // Focus ring (pill / fully rounded)
    UTFocusItem {
        target: navItem
        focusRadius: 8
    }

    // Tooltip when collapsed
    UTToolTip {
        visible: !navItem.showText && navItem.hovered && navItem.isVisible
        text: navItem.title
        cursorX: navItem.width
        cursorY: navItem.height / 2
    }
}
