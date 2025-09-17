import QtQuick 2.15

Item {
    id: root

    property color focusColor: "blue"
    property int borderWidth: 3
    property real focusRadius: NaN   // 父控件圆角
    property real focusMargin: 3    // 焦点环外扩距离
    readonly property int animationDuration: 50 // 动画时长

    width: parent ? parent.width + 2 * focusMargin : 0
    height: parent ? parent.height + 2 * focusMargin : 0
    anchors.centerIn: parent

    visible: parent ? parent.activeFocus : false
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity { 
        NumberAnimation { 
            duration: root.animationDuration; 
            easing.type: Easing.InOutQuad 
        } 
    }

    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            if (!root.visible) return

            ctx.strokeStyle = root.focusColor
            ctx.lineWidth = root.borderWidth
            ctx.setLineDash([])

            drawCTX(ctx, getCTXParam(ctx))
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onVisibleChanged: requestPaint()

        //辅助函数
        function getCTXParam(ctx) {
            var lw2 = ctx.lineWidth / 2
            var w = width - ctx.lineWidth
            var h = height - ctx.lineWidth
            var r = computeRadius(w, h) - lw2
            var isCircle = (w === h && r == w/2 - lw2)
            return {lw2: lw2, w: w, h: h, r: r, isCircle: isCircle}
        }
        
        function computeRadius(w, h) {
            var r = (!isNaN(root.focusRadius)) ? root.focusRadius + root.focusMargin : 0
            var maxR = Math.min(w, h) / 2
            return Math.max(0, Math.min(r, maxR))

        }

        function drawCircle(ctx, w, h, lw2) {
            ctx.arc(w/2, h/2, w/2 - lw2, 0, 2*Math.PI)
        }

        function drawRoundedRect(ctx, w, h, lw2, r) {
            ctx.beginPath()
            ctx.roundedRect(lw2, lw2, w, h, r, r)
            ctx.stroke()
        }

        function drawRect(ctx, w, h, lw2) {
            ctx.beginPath()
            ctx.rect(lw2, lw2, w, h)
            ctx.stroke()
        }

        function drawCTX(ctx, ctxParam) {
            if (ctxParam.isCircle) {
                drawCircle(ctx, ctxParam.w, ctxParam.h, ctxParam.lw2)
            } else if (ctxParam.r > 0) {
                drawRoundedRect(ctx, ctxParam.w, ctxParam.h, ctxParam.lw2, ctxParam.r)
            } else {
                drawRect(ctx, ctxParam.w, ctxParam.h, ctxParam.lw2)
            }
        }
    }
}
