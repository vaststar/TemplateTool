import QtQuick
import QtQuick.Controls.impl

IconImage {
    id: control
    property int iconSize: 24

    sourceSize: Qt.size(iconSize, iconSize)
    width: iconSize
    height: iconSize
    fillMode: Image.PreserveAspectFit
}
