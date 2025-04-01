import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
ApplicationWindow
{
    id: root
    MediaCameraViewController{
        id: mediaController
        objectName: "MediaCameraViewController"
    }

    visible: true
    width: 758 
    height: 576
    color: "red"

    Image {
        id: imageItem
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: mediaController.imageData
    }

    
    Component.onCompleted:{
        mediaController.showCameraImage.connect(onShowCameraImage)
    }

    
    function onShowCameraImage(img){
        console.log("rrrrrrrr")
        //  imageItem.source = img;
    }
}