import QtQuick
import QtQuick.Controls.Basic

MenuItem {
    id: control

    property int itemHeight: 32
    property int itemPadding: 12
    property int iconSpacing: 8

    height: itemHeight
    leftPadding: itemPadding
    rightPadding: itemPadding
}
