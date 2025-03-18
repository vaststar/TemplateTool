import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
Item{
    id: root
    property var controller: MainWindowSideBarController{}

// ColumnLayout {
        // anchors.fill: parent
        // spacing: 5

        ListView {
            id: naviListView
            anchors.fill: parent
            // Layout.fillWidth: true
            // Layout.fillHeight: true
            model: ListModel {
                ListElement { text: "Option 1" }
                ListElement { text: "Option 2" }
                ListElement { text: "Option 3" }
            }

            delegate: Item {
                width: parent.width
                height: parent.height/3

                Rectangle {
                    color: naviListView.currentIndex == index ? "lightblue" : "red"
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

        // Rectangle {
        //     Layout.fillHeight: true
        //     Layout.fillWidth: true
        //     color: "lightgrey"
        // }
    // }
}