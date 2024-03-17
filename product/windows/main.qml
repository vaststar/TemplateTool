import QtQuick 2.0
import QtQuick.Controls 2.0
import mylib 1.0
ApplicationWindow
{

    visible: true
    width: 640
    height: 480
    title: qsTr("Minimal Qml")
Datas{id:dd}
    FramedImage{
        id: frame
        width : 200
        height: 200
    }

}