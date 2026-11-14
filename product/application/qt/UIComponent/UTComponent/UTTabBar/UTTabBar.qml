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

    property var backgroundColorEnum: UIColorToken.Tab_Bar_Background

    property color backgroundColor: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)

    clip: false

    background: Rectangle {
        color: control.backgroundColor
    }
}
