import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

ApplicationWindow
{
    id: root
    objectName: "MainWindow"
    property MainWindowController controller: MainWindowController{}

    onClosing: function(closeEvent) {
        root.controller.onWindowClosing()
    }

    visible: root.controller.visible
    width: 1024
    height: 768
    minimumWidth: 800
    minimumHeight: 600

    readonly property bool isMacOS: Qt.platform.os === "osx"
    readonly property bool isWindows: Qt.platform.os === "windows"

    flags:
    {
        if (root.isWindows)
        {
            return Qt.Window
                   | Qt.CustomizeWindowHint
                   | Qt.WindowSystemMenuHint
                   | Qt.WindowMinimizeButtonHint
                   | Qt.WindowMaximizeButtonHint
                   | Qt.WindowCloseButtonHint
                   | Qt.ExpandedClientAreaHint
                   | Qt.NoTitleBarBackgroundHint
        }

        if (root.isMacOS)
        {
            return Qt.Window
                   | Qt.ExpandedClientAreaHint
                   | Qt.NoTitleBarBackgroundHint
        }

        return Qt.Window
    }

    // title: root.isMacOS ? "" : qsTr(root.controller.title)
    color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)

    menuBar: MainWindowMenuBar {
        id: menuBar
        visible: false
    }

    header: MainWindowTitleBar
    {
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

    MainWindowSystemTray{
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

        function onScreenChanged() {
            var r = root.controller.fitToScreen(root.x, root.y, root.width, root.height)
            root.width = r.width
            root.height = r.height
            root.x = r.x
            root.y = r.y
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
        if (root.isWindows) {
            Qt.callLater(function() {
                root.controller.applyNativeCaption(root)
            })
        }
    }
}
