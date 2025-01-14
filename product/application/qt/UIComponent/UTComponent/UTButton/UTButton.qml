import QtQuick 2.15
import QtQuick.Controls 2.15
import UIComponentBase 1.0
import UIDataStruct 1.0


BaseButton {
    id: control
    property var fontColorEnum: UIColors.UIColorsEnum_Button_Primary_Text;
    property var fontSize: UIFont.UIFontSize_ExtraLarge
    property var backgroundColorEnum: UIColors.UIColorsEnum_Button_Primary_Background;
    flat: true

    function getColor(widget, colorEnum) {
        var colorState = !widget.enabled ? UIColors.UIColorState_Disabled :
                          widget.pressed ? UIColors.UIColorState_Pressed :
                          widget.hovered ? UIColors.UIColorState_Hovered :
                          widget.activeFocus ? UIColors.UIColorState_Focused :
                          widget.checked ? UIColors.UIColorState_Checked : UIColors.UIColorState_Normal;
        return ThemeManager.getUIColor(colorEnum, colorState);
    }

    contentItem: Text{
        text: control.text
        font: ThemeManager.getUIFont(fontSize)
        color: getColor(control, fontColorEnum)//ThemeManager.getUIColor(fontColorEnum, UIColors.UIColorState_Normal);
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle{
        anchors.fill: parent
        radius: Math.min(control.height, control.width)/2
        color: getColor(control, backgroundColorEnum)//ThemeManager.getUIColor(backgroundColorEnum, UIColors.UIColorState_Normal);

        Behavior on color {
            ColorAnimation {duration: 200}
        }
    }
}