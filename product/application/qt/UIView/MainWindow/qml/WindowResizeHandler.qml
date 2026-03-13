import QtQuick

Item {
    id: resizeHandler
    anchors.fill: parent
    
    property var targetWindow
    property int borderWidth: 5
    
    // 左边缘
    MouseArea {
        width: borderWidth
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: borderWidth
        anchors.topMargin: borderWidth
        cursorShape: Qt.SizeHorCursor
        
        property real startX
        property real startWidth
        
        onPressed: (mouse) => {
            startX = targetWindow.x
            startWidth = targetWindow.width
        }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            var dx = mouse.x
            var newWidth = startWidth - dx
            targetWindow.x = startX + dx
            targetWindow.width = newWidth
        }
    }
    
    // 右边缘
    MouseArea {
        width: borderWidth
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: borderWidth
        anchors.topMargin: borderWidth
        cursorShape: Qt.SizeHorCursor
        
        property real pressedX
        property real startWidth
        
        onPressed: (mouse) => {
            pressedX = mouse.x
            startWidth = targetWindow.width
        }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            var delta = mouse.x - pressedX
            targetWindow.width = startWidth + delta
        }
    }
    
    // 上边缘
    MouseArea {
        height: borderWidth
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: borderWidth
        anchors.rightMargin: borderWidth
        cursorShape: Qt.SizeVerCursor
        
        property real startY
        property real startHeight
        
        onPressed: (mouse) => {
            startY = targetWindow.y
            startHeight = targetWindow.height
        }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            var dy = mouse.y
            var newHeight = startHeight - dy
            targetWindow.y = startY + dy
            targetWindow.height = newHeight
        }
    }
    
    // 下边缘
    MouseArea {
        height: borderWidth
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: borderWidth
        anchors.rightMargin: borderWidth
        cursorShape: Qt.SizeVerCursor
        
        property real pressedY
        property real startHeight
        
        onPressed: (mouse) => {
            pressedY = mouse.y
            startHeight = targetWindow.height
        }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            var delta = mouse.y - pressedY
            targetWindow.height = startHeight + delta
        }
    }
    
    // 左上角
    MouseArea {
        width: borderWidth
        height: borderWidth
        anchors.left: parent.left
        anchors.top: parent.top
        cursorShape: Qt.SizeFDiagCursor
        
        property real startX
        property real startY
        property real startWidth
        property real startHeight
        
        onPressed: (mouse) => {
            startX = targetWindow.x
            startY = targetWindow.y
            startWidth = targetWindow.width
            startHeight = targetWindow.height
        }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            var dx = mouse.x
            var dy = mouse.y
            targetWindow.x = startX + dx
            targetWindow.width = startWidth - dx
            targetWindow.y = startY + dy
            targetWindow.height = startHeight - dy
        }
    }
    
    // 右上角
    MouseArea {
        width: borderWidth
        height: borderWidth
        anchors.right: parent.right
        anchors.top: parent.top
        cursorShape: Qt.SizeBDiagCursor
        
        property real pressedX
        property real startY
        property real startWidth
        property real startHeight
        
        onPressed: (mouse) => {
            pressedX = mouse.x
            startY = targetWindow.y
            startWidth = targetWindow.width
            startHeight = targetWindow.height
        }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            var deltaX = mouse.x - pressedX
            targetWindow.width = startWidth + deltaX
            var dy = mouse.y
            targetWindow.y = startY + dy
            targetWindow.height = startHeight - dy
        }
    }
    
    // 左下角
    MouseArea {
        width: borderWidth
        height: borderWidth
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        cursorShape: Qt.SizeBDiagCursor
        
        property real startX
        property real pressedY
        property real startWidth
        property real startHeight
        
        onPressed: (mouse) => {
            startX = targetWindow.x
            pressedY = mouse.y
            startWidth = targetWindow.width
            startHeight = targetWindow.height
        }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            var dx = mouse.x
            targetWindow.x = startX + dx
            targetWindow.width = startWidth - dx
            var deltaY = mouse.y - pressedY
            targetWindow.height = startHeight + deltaY
        }
    }
    
    // 右下角
    MouseArea {
        width: borderWidth
        height: borderWidth
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        cursorShape: Qt.SizeFDiagCursor
        
        property real pressedX
        property real pressedY
        property real startWidth
        property real startHeight
        
        onPressed: (mouse) => {
            pressedX = mouse.x
            pressedY = mouse.y
            startWidth = targetWindow.width
            startHeight = targetWindow.height
        }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            var deltaX = mouse.x - pressedX
            var deltaY = mouse.y - pressedY
            targetWindow.width = startWidth + deltaX
            targetWindow.height = startHeight + deltaY
        }
    }
}

