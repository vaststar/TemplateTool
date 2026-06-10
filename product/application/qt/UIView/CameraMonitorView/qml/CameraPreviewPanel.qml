import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

// 2×2 camera preview wall. Each slot owns at most one MediaCameraView;
// assigning a new node id destroys-and-recreates it (one camera == one VM).
// A node is never bound in two slots simultaneously.
Item {
    id: root
    required property CameraMonitorViewController controller

    readonly property int slotCount: 4

    function _evictDuplicates(nodeId, exceptIndex) {
        if (!nodeId) return;
        for (var i = 0; i < slotCount; ++i) {
            if (i === exceptIndex) continue;
            var s = slotsRepeater.itemAt(i);
            if (s && s.currentNodeId === nodeId) {
                s.clearCamera();
            }
        }
    }

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
            id: slotsRepeater
            model: root.slotCount

            delegate: Rectangle {
                id: slot
                required property int index
                readonly property int slotIndex: index

                property string currentNodeId: ""

                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "black"
                border.width: dropArea.containsDrag ? 2 : 1
                border.color: dropArea.containsDrag ? "#80aaffaa" : "#33ffffff"
                radius: 4

                // Inline Component so the Loader resolves the cross-folder type.
                Component {
                    id: mediaCameraComponent
                    MediaCameraView {}
                }

                function clearCamera() {
                    slot.currentNodeId = "";
                    contentLoader.active = false;
                }

                function assignCamera(nodeId) {
                    if (!nodeId) return;
                    if (nodeId === slot.currentNodeId) return;
                    var src = root.controller.cameraSourceForNode(nodeId);
                    if (!src || !src.kind) return;
                    root._evictDuplicates(nodeId, slot.slotIndex);
                    slot.currentNodeId = nodeId;
                    // Tear down before re-creating so the prior VM releases first.
                    contentLoader.active = false;
                    contentLoader.active = true;
                }

                Loader {
                    id: contentLoader
                    anchors.fill: parent
                    anchors.margins: 4
                    sourceComponent: mediaCameraComponent
                    active: false
                    asynchronous: false
                }

                // Empty-slot placeholder.
                Item {
                    anchors.fill: parent
                    visible: !contentLoader.active || contentLoader.status !== Loader.Ready

                    UTText {
                        anchors.centerIn: parent
                        text: qsTr("Slot %1 — empty").arg(slot.slotIndex + 1)
                        color: "#888888"
                        fontEnum: UIFontToken.Body_Text
                    }
                }

                DropArea {
                    id: dropArea
                    anchors.fill: parent
                    keys: ["text/x-camera-node-id"]
                    onDropped: function(drop) {
                        var nodeId = drop.getDataAsString("text/x-camera-node-id");
                        if (nodeId) slot.assignCamera(nodeId);
                        drop.acceptProposedAction();
                    }
                }

                // Double-click pops the camera into a standalone window.
                // MouseArea must accept presses (default) for doubleClick to fire.
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    onDoubleClicked: (mouse) => {
                        if (slot.currentNodeId !== "") {
                            root.controller.openCameraWindow(slot.currentNodeId);
                        }
                    }
                }

                Connections {
                    target: contentLoader
                    function onLoaded() {
                        if (!contentLoader.item) return;
                        var mc = contentLoader.item.controller;
                        if (!mc) return;
                        root.controller.setupController(mc);
                        var src = root.controller.cameraSourceForNode(slot.currentNodeId);
                        if (!src || !src.kind) return;
                        if (src.kind === "local") {
                            mc.openLocalCamera(src.index);
                        } else if (src.kind === "network") {
                            mc.openNetworkCamera(src.url, src.transport,
                                                 src.openTimeoutMs, src.readTimeoutMs);
                        }
                    }
                }
            }
        }
    }
}
