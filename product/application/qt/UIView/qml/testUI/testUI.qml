import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts

Item{
    anchors.fill: parent
    // height: 800
    // length: 800
     Button {
        text: "test button"
        anchors{
            top: parent.top
            topMargin: 100
            left: parent.left
            leftMargin: 100
        }
        onClicked:{
            console.log("111111111111111")
        }
    }
}