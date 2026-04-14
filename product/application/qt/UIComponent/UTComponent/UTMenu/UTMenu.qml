import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseMenu {
    id: control

    background: Rectangle {
        implicitWidth: 200
        radius: control.borderRadius
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Menu_Background, UIColorState.Normal)
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Menu_Border, UIColorState.Normal)
        layer.enabled: true
    }
}
