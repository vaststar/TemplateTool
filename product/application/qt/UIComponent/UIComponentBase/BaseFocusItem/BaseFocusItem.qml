import QtQuick 2.15

Item {
    id: root

    property Item target: parent
    property bool externallyShown: false
    property color focusColor: "blue"
    property int borderWidth: 2
    property real focusRadius: NaN
    property real focusMargin: NaN

    readonly property real effectiveMargin: isNaN(focusMargin) ? (borderWidth / 2 + 2) : focusMargin
    readonly property int animationDuration: 50
    readonly property bool targetOk: !!target && target.visible && target.enabled
    readonly property bool targetFocused: !!target && target.activeFocus
    readonly property bool effectiveVisible: targetOk && (targetFocused || externallyShown)

    anchors.fill: target
    anchors.margins: -effectiveMargin
    anchors.centerIn: target

    focus: false
    enabled: false
    z: 9999
    visible: effectiveVisible
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity { 
        NumberAnimation { 
            duration: root.animationDuration; 
            easing.type: Easing.InOutQuad 
        } 
    }

    Canvas {
        anchors.fill: parent
        antialiasing: true
        renderTarget: Canvas.FramebufferObject
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

        function getCTXParam(ctx) {
            var lw2 = ctx.lineWidth / 2
            var w = width - ctx.lineWidth
            var h = height - ctx.lineWidth
            var r = computeRadius(w, h) - lw2
            var isCircle = (w === h && r == w/2 - lw2)
            return {lw2: lw2, w: w, h: h, r: r, isCircle: isCircle}
        }
        
        function computeRadius(w, h) {
            var r = (!isNaN(root.focusRadius)) ? root.focusRadius + root.effectiveMargin : 0
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
