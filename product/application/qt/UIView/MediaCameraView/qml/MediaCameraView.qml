import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import UIView 1.0
import UTComponent 1.0

ApplicationWindow
{
    id: root
    objectName: "MediaCameraView"

    MediaCameraViewController{
        id: mediaController
        objectName: "MediaCameraViewController"
        videoSink: videoOutput.videoSink
    }

    visible: true
    width: 758 
    height: 576
    color: "red"
    
    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectFit
        antialiasing: true
    }

    Component.onCompleted:{
        mediaController.showCameraImage.connect(onShowCameraImage)
    }
    
    function onShowCameraImage(img){
        console.log("rrrrrrrr")
    }
}