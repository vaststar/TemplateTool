import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UIDataStruct 1.0


BaseButton {
    property var colorEnum: ThemeManager.getColorSet();
    property var fontSet: UIColorSet.AAA
    property var fontEnum: ThemeManager.getColorSet().font
    // property var fon : UIColorSet.TTT
    // FontConstants{
    //     id: fontConstant
    // }
    font: ThemeManager.getFont(UIFont.Large);
    
}