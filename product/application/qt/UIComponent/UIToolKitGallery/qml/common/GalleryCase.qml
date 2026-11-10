import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

ColumnLayout {
    id: root

    property string title: ""
    property string description: ""
    spacing: 14

    UTLabel {
        Layout.fillWidth: true
        text: root.title
        fontEnum: UIFontToken.Heading_Text
        colorEnum: UIColorToken.Content_Heading
        wrapMode: Text.Wrap
    }
    UTLabel {
        Layout.fillWidth: true
        visible: root.description.length > 0
        text: root.description
        colorEnum: UIColorToken.Content_Secondary_Text
        wrapMode: Text.Wrap
    }
}
