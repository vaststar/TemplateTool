import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseMenuSeparator {
    id: control

    property var colorEnum: UIColorToken.Menu_Separator

    separatorColor: UTComponentUtil.getPlainUIColor(control.colorEnum, UIColorState.Normal)
}
