import QtQuick
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

ToolBar {
    id: root
    property MainWindowFootBarController controller: MainWindowFootBarController{}

    background: Rectangle {
        implicitHeight: 32
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Footbar_Background, UIColorState.Normal)
    }

    contentItem: UTLabel {
        text: qsTr(root.controller.footerName)
        fontEnum: UIFontToken.Body_Text
        colorEnum: UIColorToken.Footbar_Text
        horizontalAlignment: Text.AlignHCenter
    }
}
