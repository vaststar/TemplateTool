import QtQuick 2.0
import QtQuick.Controls 2.15
import UIViews 1.0

Item{
    id: root
Datas{
    id: dat
}
property var ii:"555"

function getAge()
{
    console.log("bbbbbbbbbbbbbbbbbbb")
    // console.log("aaa", root.dat.getAge())
    return "aaa"//data.getAge()
}

Label {
    id:lab
    text: "12"//dat.m_age
    font.pixelSize: 22
    font.italic: true
    Component.onCompleted:{
        console.log("done")
        // text = root.dat.getData()
    }
}

Button{
    id:butt
    text:"Button"
    height: 200
    width: 200
    x: 200
    onClicked: {
        console.log("Button clicked")
        lab.text = dat.getData()
    }
}
}

