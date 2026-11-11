import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    objectName: "utScrollViewCase"

    title: "UTScrollView"

    GalleryDemo {
        title: qsTr("Vertical scrolling")

        UTScrollView {
            id: verticalView
            objectName: "verticalScrollView"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.preferredHeight: 160

            ScrollBar.vertical: UTScrollBar {
                objectName: "verticalScrollViewBar"
                parent: verticalView
                x: verticalView.mirrored ? 0 : verticalView.width - width
                y: verticalView.topPadding
                height: verticalView.availableHeight
                active: verticalView.ScrollBar.horizontal.active
            }

            Column {
                width: verticalView.availableWidth
                spacing: 12

                UTLabel {
                    objectName: "verticalScrollContentStart"
                    width: parent.width
                    leftPadding: 8
                    rightPadding: 22
                    text: qsTr("A walk through the garden")
                    wrapMode: Text.Wrap
                    elide: Text.ElideNone
                }
                UTLabel {
                    objectName: "verticalScrollText"
                    width: parent.width
                    leftPadding: 8
                    rightPadding: 22
                    text: qsTr("The morning path winds past the herb beds, where rosemary and mint grow beside the warm stone wall. The greenhouse windows catch the early light, and the watering cans wait beneath the potting bench.\n\nBeyond the orchard, a narrow bridge crosses a quiet stream. Ferns gather along the bank, and the shade keeps the stepping stones cool even when the open meadow is bright with sunshine.\n\nThe kitchen garden changes with the seasons. In spring, young leaves fill the raised beds. Summer brings tomatoes and climbing beans, while autumn leaves a patchwork of pumpkins, seed heads, and late flowers.\n\nNear the old shed, a sheltered corner holds the tools for the next planting day. Small pots dry on the shelves, bundles of twine hang from wooden hooks, and the worktable has room for a tray of new seedlings.\n\nAt the far end of the garden, a bench faces the evening sky. The path turns back toward the house through lavender and soft grasses, with enough space to pause and enjoy the last light.")
                    wrapMode: Text.Wrap
                    elide: Text.ElideNone
                }
                UTLabel {
                    objectName: "verticalScrollContentEnd"
                    width: parent.width
                    leftPadding: 8
                    rightPadding: 22
                    bottomPadding: 8
                    text: qsTr("The garden gate marks the end of the walk.")
                    wrapMode: Text.Wrap
                    elide: Text.ElideNone
                }
            }
        }
    }

    GalleryDemo {
        title: qsTr("Horizontal and vertical scrolling")

        UTScrollView {
            id: twoWayView
            objectName: "twoWayScrollView"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 420
            Layout.preferredHeight: 160
            contentWidth: gardenGrid.implicitWidth + 30
            contentHeight: gardenGrid.implicitHeight + 30

            ScrollBar.vertical: UTScrollBar {
                id: twoWayVerticalBar
                objectName: "twoWayScrollViewVerticalBar"
                parent: twoWayView
                x: twoWayView.mirrored ? 0 : twoWayView.width - width
                y: twoWayView.topPadding
                height: Math.max(0, twoWayView.availableHeight - (twoWayHorizontalBar.visible ? twoWayHorizontalBar.height : 0))
                policy: ScrollBar.AsNeeded
                active: twoWayView.ScrollBar.horizontal.active
            }
            ScrollBar.horizontal: UTScrollBar {
                id: twoWayHorizontalBar
                objectName: "twoWayScrollViewHorizontalBar"
                parent: twoWayView
                x: twoWayView.leftPadding + (twoWayView.mirrored && twoWayVerticalBar.visible ? twoWayVerticalBar.width : 0)
                y: twoWayView.height - height
                width: Math.max(0, twoWayView.availableWidth - (twoWayVerticalBar.visible ? twoWayVerticalBar.width : 0))
                policy: ScrollBar.AsNeeded
                active: twoWayView.ScrollBar.vertical.active
            }

            Grid {
                id: gardenGrid
                x: 8
                y: 8
                columns: 3
                spacing: 12

                UTLabel {
                    objectName: "twoWayScrollContentTopLeft"
                    width: 170
                    height: 80
                    text: qsTr("Morning\nHerb beds")
                }
                UTLabel {
                    width: 170
                    height: 80
                    text: qsTr("Morning\nOrchard path")
                }
                UTLabel {
                    objectName: "twoWayScrollContentTopRight"
                    width: 170
                    height: 80
                    text: qsTr("Morning\nGreenhouse")
                }
                UTLabel {
                    width: 170
                    height: 80
                    text: qsTr("Afternoon\nFern bridge")
                }
                UTLabel {
                    width: 170
                    height: 80
                    text: qsTr("Afternoon\nKitchen garden")
                }
                UTLabel {
                    width: 170
                    height: 80
                    text: qsTr("Afternoon\nPotting shed")
                }
                UTLabel {
                    objectName: "twoWayScrollContentBottomLeft"
                    width: 170
                    height: 80
                    text: qsTr("Evening\nLavender walk")
                }
                UTLabel {
                    width: 170
                    height: 80
                    text: qsTr("Evening\nMeadow bench")
                }
                UTLabel {
                    objectName: "twoWayScrollContentBottomRight"
                    width: 170
                    height: 80
                    text: qsTr("Evening\nGarden gate")
                }
            }
        }
    }
}
