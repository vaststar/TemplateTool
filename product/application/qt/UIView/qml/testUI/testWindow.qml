import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts
import UIView 1.0

    

ApplicationWindow
{
    id: root

    TestController{
        id:controller
        objectName: "control"
    }
    visible: true
    width: 758 
    height: 576
    title: qsTr(controller.mControllerName)
    color: "steelblue"
}