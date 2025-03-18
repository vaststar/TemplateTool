import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
Item{
    id: root
    property var controller: MainWindowSideBarController{}

    ListView {
        id: naviListView
        anchors.fill: parent
        model: ListModel {
            ListElement { text: "Option 1" }
            ListElement { text: "Option 2" }
            ListElement { text: "Option 3" }
        }

        delegate: Item {
            property var isCurrentItem: ListView.isCurrentItem

            width: ListView.view.width
            height: ListView.view.height/naviListView.count

            Rectangle {
                color: isCurrentItem ? "lightblue" : "red"
                width: parent.width
                height: parent.height

                Text {
                    text: model.text
                    anchors.centerIn: parent
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("click:", index)
                        naviListView.currentIndex = index
                    }
                }
            }
        }
    }

}