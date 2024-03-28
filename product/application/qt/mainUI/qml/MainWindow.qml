import QtQuick 2.0
import QtQuick.Controls 2.0
import UIViews 1.0
// import qt.windowController 1.0

// import QtWebEngine

ApplicationWindow
{
    property var controller //: MainWindowController
    visible: true
    width: 640
    height: 480
    title: qsTr("Minimal Qml")
Datas{id:dd}
    ContactList{
        id: frame
        width : 200
        height: 200
    }

        Text {
    text: controller.mControllerName
    font.family: "Helvetica"
    font.pointSize: 24
    color: "red"
    
}

    // WebEngineView{
    //     anchors.fill:parent
    //     url:"https://www.baidu.com"
    // }
}