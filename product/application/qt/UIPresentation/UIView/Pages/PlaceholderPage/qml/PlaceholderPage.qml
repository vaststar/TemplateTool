import QtQuick
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: placeholderPage
    property string pageTitle: "未实现"

    UTText {
        anchors.centerIn: parent
        text: pageTitle
        fontEnum: UIFontToken.Subtitle_Text
        colorEnum: UIColorToken.Sidebar_Item_Text
    }
}
