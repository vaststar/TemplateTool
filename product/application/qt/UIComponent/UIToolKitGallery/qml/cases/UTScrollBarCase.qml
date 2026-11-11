import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utScrollBarCase"

    title: "UTScrollBar"

    GalleryDemo {
        title: qsTr("Arrow buttons")

        Item {
            id: arrowFrame
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 420
            Layout.preferredHeight: 174

            Flickable {
                id: arrowViewport
                objectName: "arrowScrollBarViewport"
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    rightMargin: arrowVerticalBar.width
                    bottom: parent.bottom
                    bottomMargin: arrowHorizontalBar.height
                }
                clip: true
                contentWidth: arrowGrid.implicitWidth + 16
                contentHeight: arrowGrid.implicitHeight + 16

                ScrollBar.vertical: UTScrollBar {
                    id: arrowVerticalBar
                    objectName: "arrowVerticalScrollBar"
                    parent: arrowFrame
                    anchors.top: arrowViewport.top
                    anchors.left: arrowViewport.right
                    anchors.bottom: arrowViewport.bottom
                }
                ScrollBar.horizontal: UTScrollBar {
                    id: arrowHorizontalBar
                    objectName: "arrowHorizontalScrollBar"
                    parent: arrowFrame
                    anchors.top: arrowViewport.bottom
                    anchors.left: arrowViewport.left
                    anchors.right: arrowViewport.right
                }

                Grid {
                    id: arrowGrid
                    x: 8
                    y: 8
                    columns: 3
                    spacing: 12

                    UTLabel {
                        objectName: "arrowScrollContentTopLeft"
                        width: 170
                        height: 80
                        text: qsTr("North garden\nHerb beds")
                    }
                    UTLabel {
                        width: 170
                        height: 80
                        text: qsTr("North garden\nOrchard")
                    }
                    UTLabel {
                        objectName: "arrowScrollContentTopRight"
                        width: 170
                        height: 80
                        text: qsTr("North garden\nGreenhouse")
                    }
                    UTLabel {
                        width: 170
                        height: 80
                        text: qsTr("Central garden\nFern bridge")
                    }
                    UTLabel {
                        width: 170
                        height: 80
                        text: qsTr("Central garden\nVegetable beds")
                    }
                    UTLabel {
                        width: 170
                        height: 80
                        text: qsTr("Central garden\nPotting shed")
                    }
                    UTLabel {
                        objectName: "arrowScrollContentBottomLeft"
                        width: 170
                        height: 80
                        text: qsTr("South garden\nLavender walk")
                    }
                    UTLabel {
                        width: 170
                        height: 80
                        text: qsTr("South garden\nMeadow bench")
                    }
                    UTLabel {
                        objectName: "arrowScrollContentBottomRight"
                        width: 170
                        height: 80
                        text: qsTr("South garden\nGarden gate")
                    }
                }
            }
        }
    }

    GalleryDemo {
        title: qsTr("Without arrow buttons")

        Item {
            id: noArrowFrame
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 420
            Layout.preferredHeight: 160

            Flickable {
                id: noArrowViewport
                objectName: "noArrowScrollBarViewport"
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    rightMargin: noArrowVerticalBar.width
                    bottom: parent.bottom
                }
                clip: true
                contentWidth: width
                contentHeight: trailNotes.implicitHeight
                flickableDirection: Flickable.VerticalFlick

                ScrollBar.vertical: UTScrollBar {
                    id: noArrowVerticalBar
                    objectName: "noArrowVerticalScrollBar"
                    parent: noArrowFrame
                    anchors.top: noArrowViewport.top
                    anchors.left: noArrowViewport.right
                    anchors.bottom: noArrowViewport.bottom
                    showArrows: false
                }

                Column {
                    id: trailNotes
                    width: noArrowViewport.width
                    spacing: 12

                    UTLabel {
                        objectName: "noArrowScrollContentStart"
                        width: parent.width
                        padding: 8
                        text: qsTr("Woodland trail\nThe path starts beneath the old oak tree and follows the stream toward a shaded fern garden.")
                        wrapMode: Text.Wrap
                        elide: Text.ElideNone
                    }
                    UTLabel {
                        width: parent.width
                        padding: 8
                        text: qsTr("Meadow crossing\nWildflowers line the open path. A quiet bench looks across the grasses toward the orchard.")
                        wrapMode: Text.Wrap
                        elide: Text.ElideNone
                    }
                    UTLabel {
                        width: parent.width
                        padding: 8
                        text: qsTr("Orchard turn\nApple trees shade the lane beside the potting shed. The greenhouse stands beyond the kitchen garden.")
                        wrapMode: Text.Wrap
                        elide: Text.ElideNone
                    }
                    UTLabel {
                        objectName: "noArrowScrollContentEnd"
                        width: parent.width
                        padding: 8
                        text: qsTr("Garden gate\nThe trail ends beside the lavender walk, where the last light falls across the stone wall.")
                        wrapMode: Text.Wrap
                        elide: Text.ElideNone
                    }
                }
            }
        }
    }
}
