import QtQuick 2.15

Item {
    id: root

    property color focusColor: "blue"
    property int borderWidth: 3
    property real focusRadius: NaN   // 父控件圆角
    property real focusMargin: 4    // 焦点环外扩距离

    width: parent ? parent.width + 2 * focusMargin : 0
    height: parent ? parent.height + 2 * focusMargin : 0
    visible: parent ? parent.activeFocus : false

    anchors.centerIn: parent

    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            if (!root.visible) return

            ctx.strokeStyle = root.focusColor
            ctx.lineWidth = root.borderWidth
            ctx.setLineDash([])

            var lw2 = ctx.lineWidth / 2
            var w = parent.width - ctx.lineWidth
            var h = parent.height - ctx.lineWidth

            var r = (!isNaN(root.focusRadius)) ? root.focusRadius + root.focusMargin : 0
            if (r >= h/2) r = h/2 - 1

            // 圆形
            if (parent.width === parent.height && parent.width > 0) {
                ctx.beginPath()
                ctx.arc(parent.width/2, parent.height/2, parent.width/2 - lw2, 0, 2*Math.PI)
                ctx.stroke()
            }
            // 圆角矩形
            else if (r > 0) {
                ctx.beginPath()
                ctx.moveTo(lw2 + r, lw2)
                ctx.lineTo(w - r + lw2, lw2)
                ctx.arcTo(w + lw2, lw2, w + lw2, r + lw2, r)
                ctx.lineTo(w + lw2, h - r + lw2)
                ctx.arcTo(w + lw2, h + lw2, w - r + lw2, h + lw2, r)
                ctx.lineTo(lw2 + r, h + lw2)
                ctx.arcTo(lw2, h + lw2, lw2, h - r + lw2, r)
                ctx.lineTo(lw2, r + lw2)
                ctx.arcTo(lw2, lw2, lw2 + r, lw2, r)
                ctx.closePath()
                ctx.stroke()
            }
            // 普通矩形
            else {
                ctx.strokeRect(lw2, lw2, w, h)
            }
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onVisibleChanged: requestPaint()
    }
}
