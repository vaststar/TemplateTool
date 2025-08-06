// WindowAnimator.qml
import QtQuick
import QtQuick.Controls
import QtGraphicalEffects

QtObject {
    id: animator

    // 需要绑定的 ApplicationWindow 或 QQuickWindow 对象
    // property alias appWindow: snapshot.sourceItem

    // 截图源，绑定到 appWindow.contentItem
// property Item targetItem: appWindow ? appWindow.contentItem : null

// ShaderEffectSource {
//     id: snapshot
//     sourceItem: targetItem !== null ? targetItem : undefined
//     live: false
//     hideSource: false
// }



    // 动画覆盖层，显示截图并模糊
    Image {
        id: animatedLayer
        anchors.fill: parentWindow
        visible: false
        opacity: 0
        scale: 1
        layer.enabled: true
        layer.smooth: true
        layer.effect: FastBlur {
            radius: 15
            source: animatedLayer
        }
    }

    // 绑定窗口的根元素，为了设置动画覆盖层大小
    property Item parentWindow: null
    required property ApplicationWindow appWindow

    function startAnimation(action) {
        if (!appWindow || !parentWindow)
            return

        // snapshot.sourceItem = appWindow
        // snapshot.grab()
        // animatedLayer.visible = true
        // animatedLayer.opacity = 1
        // animatedLayer.scale = 1

        // anim.action = action
        // anim.start()
    }

    function minimize() {
        startAnimation("minimize")
    }

    function toggleMaximize() {
        startAnimation("toggleMaximize")
    }

    // SequentialAnimation {
    //     id: anim
    //     property string action: ""

    //     PropertyAnimation { target: animatedLayer; property: "scale"; to: 0.8; duration: 300 }
    //     PropertyAnimation { target: animatedLayer; property: "opacity"; to: 0; duration: 300 }
    //     ScriptAction {
    //         script: {
    //             animatedLayer.visible = false
    //             if (anim.action === "minimize")
    //                 appWindow.showMinimized()
    //             else if (anim.action === "toggleMaximize") {
    //                 if (appWindow.visibility === ApplicationWindow.Maximized)
    //                     appWindow.showNormal()
    //                 else
    //                     appWindow.showMaximized()
    //             }
    //         }
    //     }
    // }
}
