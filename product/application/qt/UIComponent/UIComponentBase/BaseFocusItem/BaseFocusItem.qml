import QtQuick

Item {
    id: focusHighlight
    property var focusColor: "blue"
    property var focusWidth: 2
    property var focusRadius: 0
    property var focusMargins: -2


    visible: parent.activeFocus
    anchors.fill: parent
    anchors.margins: focusMargins

    // 绘制焦点框
    Rectangle {
        id: focusRect
        smooth: true
        anchors.fill: parent
        border.color: focusHighlight.focusColor
        border.width: focusHighlight.focusWidth
        color: "transparent"
        radius: focusHighlight.focusRadius
    }
}