import QtQuick
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTTabBar - Themed TabBar with UT design tokens.
 *
 * Usage:
 *   UTTabBar {
 *       UTTabButton { text: "Tab 1" }
 *       UTTabButton { text: "Tab 2" }
 *   }
 */
BaseTabBar {
    id: control

    property var backgroundColorEnum: UIColorToken.Content_Section_Background
    property var indicatorColorEnum: UIColorToken.Button_Primary_Background

    background: Rectangle {
        color: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)

        // Bottom indicator line under the selected tab
        Rectangle {
            id: tabIndicator
            height: 3
            radius: 1.5
            color: UTComponentUtil.getPlainUIColor(control.indicatorColorEnum, UIColorState.Normal)
            anchors.bottom: parent.bottom

            // Track the current tab's position and width
            x: control.currentItem ? control.currentItem.x : 0
            width: control.currentItem ? control.currentItem.width : 0

            Behavior on x { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
            Behavior on width { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
        }
    }
}
