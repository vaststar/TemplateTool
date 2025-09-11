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
    property var radius: Math.min(control.height, control.width)/2
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
        color: UTComponentUtil.getItemUIColor(control, backgroundColorEnum)

        Behavior on color {
            ColorAnimation {duration: 250}
        }
    }

    UTFocusItem{
        focusRadius: control.radius
    }
}