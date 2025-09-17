import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseButton {
    id: control
    property var fontColorEnum: UIColorToken.ButtonPrimaryText;
    property var fontSize: UIFontToken.UIFontSize_Small
    property var backgroundColorEnum: UIColorToken.ButtonPrimaryBackground;
    property var borderColorEnum: UIColorToken.ButtonPrimaryBorder;
    property var radius: Math.min(control.height, control.width)/2
    property var borderWidth: 1
    clip: false

    contentItem: Text{
        text: control.text
        font: UTComponentUtil.getUIFont(fontSize)
        color: UTComponentUtil.getItemUIColor(control, fontColorEnum)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle{
        anchors.fill: parent
        radius: control.radius
        border.width: control.borderWidth
        color: UTComponentUtil.getItemUIColor(control, backgroundColorEnum)
        border.color: UTComponentUtil.getItemUIColor(control, borderColorEnum)

        Behavior on color {
            ColorAnimation {duration: 150}
        }
    }

    UTFocusItem{
        focusRadius: control.radius
    }
}