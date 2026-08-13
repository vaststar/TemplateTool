import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

// Label + value row used throughout the detail panel. Auto-hides when value is empty
// so each body just lists every potential field without having to gate visibility.
RowLayout {
    id: root
    property string label
    property string value

    Layout.fillWidth: true
    spacing: 12
    visible: value !== undefined && value !== ""

    UTText {
        text: root.label
        Layout.preferredWidth: 72
        fontEnum: UIFontToken.Body_Text
        colorEnum: UIColorToken.Sidebar_Item_Text
    }
    UTText {
        text: root.value || ""
        Layout.fillWidth: true
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        fontEnum: UIFontToken.Body_Text
        colorEnum: UIColorToken.Content_Text
    }
}
