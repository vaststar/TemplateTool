import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UIElementData 1.0
import UTComponent 1.0


BaseButton {
    id: control
    property var fontColorEnum: UIElementData.UIColorEnum_Button_Primary_Text;
    property var fontSize: UIElementData.UIFontSize_Small
    property var backgroundColorEnum: UIElementData.UIColorEnum_Button_Primary_Background;
    property var radius: Math.min(control.height, control.width)/2

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