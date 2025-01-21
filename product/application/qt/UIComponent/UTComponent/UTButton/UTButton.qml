import QtQuick 2.15
import QtQuick.Controls 2.15
import UIComponentBase 1.0
import UIElementData 1.0


BaseButton {
    id: control
    property var fontColorEnum: UIElementData.UIColorEnum_Button_Primary_Text;
    property var fontSize: UIElementData.UIFontSize_ExtraLarge
    property var backgroundColorEnum: UIElementData.UIColorEnum_Button_Primary_Background;
    flat: true

    function getColor(widget, colorEnum) {
        var colorState = !widget.enabled ? UIElementData.UIColorState_Disabled :
                          widget.pressed ? UIElementData.UIColorState_Pressed :
                          widget.hovered ? UIElementData.UIColorState_Hovered :
                          widget.activeFocus ? UIElementData.UIColorState_Focused :
                          widget.checked ? UIElementData.UIColorState_Checked : UIElementData.UIColorState_Normal;
        return ThemeManager.getUIColor(colorEnum, colorState);
    }

    contentItem: Text{
        text: control.text
        font: ThemeManager.getUIFont(fontSize)
        color: getColor(control, fontColorEnum)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle{
        anchors.fill: parent
        radius: Math.min(control.height, control.width)/2
        color: getColor(control, backgroundColorEnum)

        Behavior on color {
            ColorAnimation {duration: 200}
        }
    }
}