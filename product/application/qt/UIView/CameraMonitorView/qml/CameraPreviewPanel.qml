import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

// 2×2 摄像机预览栅格。
// 每个 cell 是一个占位槽位，未来可在 contentLoader 里 setSource 一个
// Item-rooted 的 MediaCameraView 子组件来真正拉流。
// CameraMonitorViewController 不直接持有视频通道——视频解码归各个 MediaCameraView 自治。
Item {
    id: root
    required property CameraMonitorViewController controller

    readonly property int slotCount: 4

    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 8
        columns: 2
        rows: 2
        rowSpacing: 8
        columnSpacing: 8

        Repeater {
            model: root.slotCount
            delegate: Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "black"
                border.width: 1
                border.color: "#33ffffff"
                radius: 4

                // Future hook: setSource("MediaCameraView.qml", { cameraId: ... })
                Loader {
                    id: contentLoader
                    anchors.fill: parent
                    anchors.margins: 4
                    source: ""
                }

                // Placeholder content shown while the slot is empty.
                Item {
                    anchors.fill: parent
                    visible: contentLoader.status !== Loader.Ready

                    UTText {
                        anchors.centerIn: parent
                        text: qsTr("Slot %1 — empty").arg(index + 1)
                        color: "#888888"
                        fontEnum: UIFontToken.Body_Text
                    }
                }
            }
        }
    }
}
