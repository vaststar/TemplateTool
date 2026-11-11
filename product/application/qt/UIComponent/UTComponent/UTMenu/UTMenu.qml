import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

BaseMenu {
    id: control

    property var backgroundColorEnum: UIColorToken.Menu_Background
    property var borderColorEnum: UIColorToken.Menu_Border

    backgroundColor: UTComponentUtil.getPlainUIColor(control.backgroundColorEnum, UIColorState.Normal)
    borderColor: UTComponentUtil.getPlainUIColor(control.borderColorEnum, UIColorState.Normal)

    delegate: UTMenuItem {}
}
