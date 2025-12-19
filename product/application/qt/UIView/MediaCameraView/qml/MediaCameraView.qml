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

    // visible: true
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

    Component.onCompleted:{
        controller.logInfo("onCompleted")
        controller.videoSink = videoOutput.videoSink
        controller.showCameraImage.connect(onShowCameraImage)
        ControllerInitializer.initializeController(controller)
    }
    
    function onShowCameraImage(img){
        // controller.logInfo("MediaCameraView received image")
    }
}