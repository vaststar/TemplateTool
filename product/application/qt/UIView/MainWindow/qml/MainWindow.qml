import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

ApplicationWindow
{
    id: root
    property MainWindowController controller: MainWindowController{}

    visible: root.controller.visible
    width: 1024
    height: 768
    minimumWidth: 800
    minimumHeight: 600
    flags: Qt.FramelessWindowHint | Qt.Window

    title: qsTr(root.controller.title)
    color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)

    Rectangle {
        parent: Overlay.overlay
        anchors.fill: parent
        z: 9999
        color: "transparent"
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(UIColorToken.Window_Border, UIColorState.Normal)
    }

    menuBar: MainWindowMenuBar {
        id: menuBar
        visible: false
    }

    header: MainWindowTitleBar {
        id: titleBar
        appWindow: root
        height: 40
    }

    footer: MainWindowFootBar {
        id: footBar
    }

    WindowResizeHandler {
        parent: Overlay.overlay
        z: 10000
        targetWindow: root
        borderWidth: 5
    }

    Loader{
        id: mainWindowContentLoader
        anchors.fill: parent
        anchors.margins: 1
        focus: true
    }

    AppSystemTray{
        id: systemTray
    }

    Connections {
        target: root.controller

        function onControllerInitialized() {
            onMainControllerInitialized()
        }

        function onActivateWindow() {
            showAndActivate()
        }

        function onHideWindow() {
            root.hide()
        }

        function onShowWindow() {
            showAndActivate()
        }

        function onMinimizeWindow() {
            root.showMinimized()
        }
    }

    function onMainControllerInitialized(){
        if(!root.controller){
            console.log("MainWindow controller is null")
            return
        }
        root.controller.logInfo("MainWindowController onMainControllerInitialized")
        root.controller.setupController(menuBar.controller)
        root.controller.setupController(titleBar.controller)
        root.controller.setupController(footBar.controller)
        root.controller.setupController(systemTray.controller)
        mainWindowContentLoader.setSource("MainWindowContent.qml",{"controller":root.controller});
        root.controller.componentCompleted()
    }

    function showAndActivate() {
        root.show()
        root.visibility = Window.Windowed
        root.raise()
        root.requestActivate()
    }

    Component.onCompleted:{
        root.controller.logInfo("MainWindow QML Component onCompleted")
    }
}
