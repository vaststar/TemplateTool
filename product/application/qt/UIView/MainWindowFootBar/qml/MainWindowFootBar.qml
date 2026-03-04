import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

ToolBar {
    id: root
    property MainWindowFootBarController controller: MainWindowFootBarController{}

    background: Rectangle {
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Footbar_Background, UIColorState.Normal)
    }

    RowLayout {
        anchors.fill: parent
        ToolButton {
            text: qsTr("‹")
            contentItem: Text { text: parent.text; color: UTComponentUtil.getPlainUIColor(UIColorToken.Footbar_Text, UIColorState.Normal); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            onClicked: stack.pop()
        }
        Label {
            text: qsTr(root.controller.footerName)
            color: UTComponentUtil.getPlainUIColor(UIColorToken.Footbar_Text, UIColorState.Normal)
            elide: Label.ElideRight
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: true
        }
        ToolButton {
            text: qsTr("⋮")
            contentItem: Text { text: parent.text; color: UTComponentUtil.getPlainUIColor(UIColorToken.Footbar_Text, UIColorState.Normal); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
            onClicked: menu.open()
        }
    }
}
