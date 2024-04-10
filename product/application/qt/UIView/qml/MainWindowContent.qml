import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts
import UIView 1.0

Item{
    required property var controller
    
    StackView {
        anchors.fill: parent
        initialItem: Rectangle {
            width: 400
            height: 200
            color: "salmon"
        }
    }
    ContactList{
        id: frame
        width : 400
        height: 200
    }

    Text {
        text: controller.mControllerName + "test"
        font.family: "Helvetica"
        font.pointSize: 12
        color: "red"
        anchors{
            top: parent.top
            topMargin: 200
            left: parent.left
            leftMargin: 200
        }
    }
}