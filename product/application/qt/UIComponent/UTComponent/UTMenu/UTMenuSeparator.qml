import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseMenuSeparator {
    id: control

    contentItem: Rectangle {
        implicitHeight: 1
        implicitWidth: parent ? parent.width : 100
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Menu_Separator, UIColorState.Normal)
    }
}
