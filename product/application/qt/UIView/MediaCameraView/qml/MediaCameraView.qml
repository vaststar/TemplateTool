import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import UIView 1.0
import UTComponent 1.0

ApplicationWindow
{
    id: root
    property MediaCameraViewController controller: MediaCameraViewController{
    }

    width: 758 
    height: 576
    color: "red"
    visible: controller.visible
    
    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectFit
        antialiasing: true
    }

    Connections {
        target: controller
        
        function onControllerInitialized() {
            cameraHandlers.onControllerInitialized()
        }

        function onShowCameraImage(img) {
            cameraHandlers.onShowCameraImage(img)
        }
    }

    Component.onCompleted:{
        controller.logInfo("qml load onCompleted")
        controller.videoSink = videoOutput.videoSink
        // ControllerInitializer.initializeController(controller)
    }

    QtObject {
        id: cameraHandlers
        
        function onControllerInitialized() {
            controller.logInfo("MediaCameraView controller initialized")
        }

        function onShowCameraImage(img) {
            controller.logInfo("Camera frame received")
        }
    }
}