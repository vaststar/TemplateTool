import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UIElementData 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

ApplicationWindow
{
    id: root

    MainWindowController{
        id: mainController
        objectName: "MainWindowController"
    }

    visible: mainController.visible
    width: mainController ? mainController.width : 100
    height: mainController? mainController.height : 100
    flags: Qt.FramelessWindowHint|Qt.Window

    title: qsTr(mainController.title)
    color: UTComponentUtil.getPlainUIColor(UIColorToken.MainWindowBackground, UIColorState.Normal)

    menuBar: MainWindowMenuBar {
        id: menuBarId
        visible: false
    }

    header: MainWindowTitleBar {
        id: titleBar
        appWindow: root
    }

    footer: MainWindowFootBar {
        id: footBar
    }

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
        mainController.onInitTitleBarController(titleBar.controller)
        mainController.onInitFootBarController(footBar.controller)
        mainController.onInitSystemTrayController(systemTray.controller)
        mainWindowContentLoader.setSource("MainWindowContent.qml",{"controller":mainController});
    }

    AppSystemTray{
        id: systemTray
    }
}