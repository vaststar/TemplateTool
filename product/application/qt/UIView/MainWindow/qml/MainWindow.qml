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
    property MainWindowController controller: MainWindowController{}

    visible: root.controller.visible
    width: root.controller ? root.controller.width : 100
    height: root.controller ? root.controller.height : 100
    flags: Qt.FramelessWindowHint|Qt.Window

    title: qsTr(root.controller.title)
    color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)

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

    Loader{
        id: mainWindowContentLoader
        anchors.fill: parent
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
            onShowActivateWindow()
        }
    }

    function onMainControllerInitialized(){
        if(!root.controller){
            console.log("MainWindow controller is null")
            return
        }
        root.controller.logInfo("MainWindowController onMainControllerInitialized")
        root.controller.initController(menuBar.controller)
        root.controller.initController(titleBar.controller)
        root.controller.initController(footBar.controller)
        root.controller.initController(systemTray.controller)
        mainWindowContentLoader.setSource("MainWindowContent.qml",{"controller":root.controller});
        root.controller.componentCompleted()
    }

    function onShowActivateWindow() {
        root.show()
        root.visibility = Window.Windowed
        root.raise()
        root.requestActivate()
    }
    
    Component.onCompleted:{
        root.controller.logInfo("MainWindow QML Component onCompleted")
    }
}