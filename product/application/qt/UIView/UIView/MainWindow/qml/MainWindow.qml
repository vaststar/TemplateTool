import QtQuick
import QtQuick.Controls
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
    title: mainController.title
    color: "steelblue"
    menuBar: AppMenuBar {
        id: menuBarId
    }
    header: AppHeader {
        visible: false
    }
    footer: AppFooter {}

    Loader{
        id: mainWindowContentLoader
        anchors.fill: parent
        focus: true
    }

    Component.onCompleted:{
        mainController.controllerInitialized.connect(onMainControllerInitialized)
    }


    function onMainControllerInitialized(){
        mainController.onInitMenuBarController(menuBarId.controller)
        mainWindowContentLoader.setSource("MainWindowContent.qml",{"controller":mainController});
    }

    SystemTray{
        id: systemTray
    }
}