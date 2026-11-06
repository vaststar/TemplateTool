import QtQuick
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root

    implicitWidth: 1
    property real verticalInset: 16

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: root.verticalInset
        anchors.bottomMargin: root.verticalInset
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border, UIColorState.Normal)
        opacity: 0.65
    }
}
