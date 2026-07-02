import QtQuick
import UTComponent 1.0
import UIResourceLoader 1.0

Window {
    id: hostDialog

    // Injected at construction time by createQmlWindow(initialProperties).
    required property string appId
    required property string appName

    width: 900
    height: 640
    minimumWidth: 480
    minimumHeight: 360
    title: qsTr("Mini App - ") + appName
    color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Background, UIColorState.Normal)

    // Header
    Rectangle {
        id: header
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 48
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Item_Background, UIColorState.Normal)

        UTText {
            anchors { verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 16 }
            text: hostDialog.appName
            fontEnum: UIFontToken.Subtitle_Text
            colorEnum: UIColorToken.Sidebar_Item_Text
        }
    }

    // Content placeholder (WebEngineView will live here in a later stage).
    UTText {
        anchors.centerIn: parent
        text: qsTr("Content area (WebEngine to be integrated) - ") + hostDialog.appId
        fontEnum: UIFontToken.Subtitle_Text
        colorEnum: UIColorToken.Sidebar_Item_Text
    }
}
