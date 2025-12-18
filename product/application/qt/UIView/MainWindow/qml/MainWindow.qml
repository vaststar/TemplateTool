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
        root.controller.controllerInitialized.connect(onMainControllerInitialized)
        root.controller.onComponentCompleted();
        ControllerInitializer.initializeController(root.controller)
    }

    function onMainControllerInitialized(){
        mainWindowContentLoader.setSource("MainWindowContent.qml",{"controller":root.controller});
    }

    AppSystemTray{
        id: systemTray
    }
}