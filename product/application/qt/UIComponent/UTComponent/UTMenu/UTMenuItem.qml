import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseMenuItem {
    id: control

    palette.windowText: UTComponentUtil.getItemUIColor(control, UIColorToken.Menu_Text)

    contentItem: RowLayout {
        spacing: control.iconSpacing
        LayoutMirroring.enabled: false
        layoutDirection: control.mirrored ? Qt.RightToLeft : Qt.LeftToRight

        Item {
            visible: control.checkable && control.indicator
            implicitWidth: control.indicator ? control.indicator.width : 0
        }

        UTText {
            Layout.fillWidth: true
            text: control.text
            fontEnum: UIFontToken.Body_Text
            color: UTComponentUtil.getItemUIColor(control, UIColorToken.Menu_Text)
        }

        UTText {
            visible: control.action && control.action.shortcut
            text: control.action ? control.action.shortcut : ""
            fontEnum: UIFontToken.Caption_Text
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Menu_Shortcut_Text, UIColorState.Normal)
        }

        Item {
            visible: control.subMenu && control.arrow
            implicitWidth: control.arrow ? control.arrow.width : 0
        }
    }

    background: Rectangle {
        id: _bg
        radius: 4
        color: control.highlighted || control.hovered
              ? UTComponentUtil.getPlainUIColor(UIColorToken.Menu_Item_Background, UIColorState.Hovered)
              : UTComponentUtil.getPlainUIColor(UIColorToken.Menu_Item_Background, UIColorState.Normal)

        Behavior on color { ColorAnimation { duration: 100 } }

        UTFocusItem {
            target: _bg
            delegateFocused: control.highlighted
            focusRadius: _bg.radius
            focusMargin: -1
        }
    }
}
