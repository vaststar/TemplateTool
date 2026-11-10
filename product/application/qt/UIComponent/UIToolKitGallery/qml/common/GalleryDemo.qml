import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

Rectangle {
    id: root

    default property alias content: samples.data
    property string title: ""
    property string note: ""

    Layout.fillWidth: true
    implicitHeight: column.implicitHeight + 24
    radius: 8
    color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    border.width: 1
    border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border, UIColorState.Normal)

    ColumnLayout {
        id: column
        x: 12
        y: 12
        width: Math.max(0, root.width - 24)
        spacing: 10

        UTLabel {
            Layout.fillWidth: true
            text: root.title
            fontEnum: UIFontToken.Body_Text_Medium
            colorEnum: UIColorToken.Content_Section_Title
            wrapMode: Text.Wrap
        }
        UTLabel {
            Layout.fillWidth: true
            visible: root.note.length > 0
            text: root.note
            colorEnum: UIColorToken.Content_Secondary_Text
            wrapMode: Text.Wrap
        }
        ColumnLayout {
            id: samples
            Layout.fillWidth: true
            spacing: 10
        }
    }
}
