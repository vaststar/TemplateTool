import QtQuick 2.15

Item {
    id: root

    property alias text: generator.text
    property alias errorLevel: generator.errorLevel
    property alias border: generator.border
    property alias darkColor: generator.darkColor
    property alias lightColor: generator.lightColor
    property alias valid: generator.valid

    property int displaySize: 200
    property bool antialiasing: true

    implicitWidth: displaySize
    implicitHeight: displaySize

    QRCodeGenerator {
        id: generator
    }

    Image {
        id: qrImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        smooth: root.antialiasing
        antialiasing: root.antialiasing
        source: generator.valid ? "data:image/svg+xml;utf8," + generator.svgData : ""
        visible: generator.valid

        Behavior on opacity {
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }

    Text {
        anchors.centerIn: parent
        text: "Invalid QR Code"
        color: "#999999"
        font.pixelSize: 14
        visible: !generator.valid && root.text !== ""
    }
}
