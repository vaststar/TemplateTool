import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import UTComponent 1.0
import "../common"

GalleryCase {
    id: root
    objectName: "utWindowCase"

    title: "UTWindow"

    property var demoWindow: null
    property bool _destroying: false

    function openWindow() {
        if (demoWindow) {
            demoWindow.raise()
            demoWindow.requestActivate()
            return
        }
        demoWindow = windowComponent.createObject(root, {
            "transientParent": root.Window.window
        })
    }

    function closeWindow() {
        if (demoWindow)
            demoWindow.close()
    }

    onVisibleChanged: {
        if (!visible)
            closeWindow()
    }
    Component.onDestruction: {
        _destroying = true
        closeWindow()
    }

    Timer {
        id: restoreFocus
        interval: 0
        onTriggered: {
            if (root.visible && !root._destroying) {
                if (root.Window.window)
                    root.Window.window.requestActivate()
                openWindowButton.forceActiveFocus(Qt.OtherFocusReason)
            }
        }
    }

    GalleryDemo {
        title: qsTr("Application window")

        UTButton {
            id: openWindowButton
            objectName: "openWindowButton"
            text: qsTr("Open window")
            onClicked: root.openWindow()
        }
    }

    Component {
        id: windowComponent

        UTWindow {
            id: previewWindow
            objectName: "demoWindow"
            title: qsTr("Project workspace")
            width: 480
            height: 280
            minimumWidth: 360
            minimumHeight: 220
            x: transientParent ? transientParent.x + (transientParent.width - width) / 2 : 0
            y: transientParent ? transientParent.y + (transientParent.height - height) / 2 : 0

            onClosing: {
                root.demoWindow = null
                if (root.visible && !root._destroying)
                    restoreFocus.start()
                destroy()
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 12

                UTLabel {
                    text: qsTr("Workspace")
                }
                UTText {
                    Layout.fillWidth: true
                    text: qsTr("UIComponent Gallery")
                    wrapMode: Text.Wrap
                }
                Item {
                    Layout.fillHeight: true
                }
                UTButton {
                    objectName: "closeWindowButton"
                    Layout.alignment: Qt.AlignRight
                    text: qsTr("Close")
                    focus: true
                    onClicked: previewWindow.close()
                }
            }
        }
    }
}
