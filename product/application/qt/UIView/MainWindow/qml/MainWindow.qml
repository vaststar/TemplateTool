import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts
import UIView 1.0

ApplicationWindow
{
    id: root

    MainWindowController{
        id:mainController
        objectName: "MainWindowController"
    }

    visible: true
    width: 758 
    height: 576
    title: qsTr(mainController.mTitle)
    color: "steelblue"
    menuBar: AppMenuBar {}
    header: AppHeader {
        visible: false
    }
    footer: AppFooter {}

    Loader{
        id: mainWindowContentLoader
        anchors.fill: parent
    }

    Component.onCompleted:{
        mainController.controllerInitialized.connect(onMainControllerInitialized)
    }


    function onMainControllerInitialized( appContext ){
        mainWindowContentLoader.setSource("MainWindowContent.qml",{"controller":mainController});
    }

    SystemTray{
        id: systemTray
    }
}