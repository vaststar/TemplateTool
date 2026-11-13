import QtQuick 2.15

import UIComponentBase 1.0
Item {
    id: root

    property alias text: generator.text
    property alias errorLevel: generator.errorLevel
    property alias border: generator.border
    property alias darkColor: generator.darkColor
    property alias lightColor: generator.lightColor
    property alias valid: generator.valid

    property int displaySize: 200
    // The base owns invalid-state visibility; wrappers supply presentation.
    property Component invalidIndicator: null

    implicitWidth: displaySize
    implicitHeight: displaySize

    QRCodeGenerator {
        id: generator
    }

    Image {
        id: qrImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        smooth: false
        antialiasing: false
        mipmap: false
        source: generator.valid ? "data:image/svg+xml;utf8," + generator.svgData : ""
        visible: generator.valid
        sourceSize: Qt.size(displaySize*4, displaySize*4)

        Behavior on opacity {
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }

    Loader {
        anchors.centerIn: parent
        active: !generator.valid && root.text !== ""
        sourceComponent: root.invalidIndicator
    }
}
