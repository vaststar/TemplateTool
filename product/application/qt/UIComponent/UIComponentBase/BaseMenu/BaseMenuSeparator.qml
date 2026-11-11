import QtQuick
import QtQuick.Controls.Basic

MenuSeparator {
    id: control

    property int separatorMargin: 4
    property color separatorColor: palette.mid

    leftPadding: separatorMargin
    rightPadding: separatorMargin

    contentItem: Rectangle {
        implicitHeight: 1
        color: control.separatorColor
    }
}
