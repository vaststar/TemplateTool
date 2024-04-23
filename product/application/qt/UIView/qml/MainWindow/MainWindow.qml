import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts
import UIView 1.0

ApplicationWindow
{
    id: root
    MainWindowController{
        id:mainController
    }

    visible: true
    width: 758 
    height: 576
    title: qsTr("TIAN TOOL")
    color: "steelblue"
    menuBar: AppMenuBar {}
    header: AppHeader {
        visible: false
    }
    footer: AppFooter {}

    MainWindowContent{
        id: mainWindowContent
        controller: mainController
        anchors.fill: parent
        
    }
}