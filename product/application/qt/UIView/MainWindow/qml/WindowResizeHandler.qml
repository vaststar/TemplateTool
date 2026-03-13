import QtQuick
import QtQuick.Window

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
        hoverEnabled: true
        onPressed: targetWindow.controller.startSystemResize(targetWindow, Qt.LeftEdge)
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
        hoverEnabled: true
        onPressed: targetWindow.controller.startSystemResize(targetWindow, Qt.RightEdge)
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
        hoverEnabled: true
        onPressed: targetWindow.controller.startSystemResize(targetWindow, Qt.TopEdge)
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
        hoverEnabled: true
        onPressed: targetWindow.controller.startSystemResize(targetWindow, Qt.BottomEdge)
    }

    // 左上角
    MouseArea {
        width: borderWidth
        height: borderWidth
        anchors.left: parent.left
        anchors.top: parent.top
        cursorShape: Qt.SizeFDiagCursor
        hoverEnabled: true
        onPressed: targetWindow.controller.startSystemResize(targetWindow, Qt.LeftEdge | Qt.TopEdge)
    }

    // 右上角
    MouseArea {
        width: borderWidth
        height: borderWidth
        anchors.right: parent.right
        anchors.top: parent.top
        cursorShape: Qt.SizeBDiagCursor
        hoverEnabled: true
        onPressed: targetWindow.controller.startSystemResize(targetWindow, Qt.RightEdge | Qt.TopEdge)
    }

    // 左下角
    MouseArea {
        width: borderWidth
        height: borderWidth
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        cursorShape: Qt.SizeBDiagCursor
        hoverEnabled: true
        onPressed: targetWindow.controller.startSystemResize(targetWindow, Qt.LeftEdge | Qt.BottomEdge)
    }

    // 右下角
    MouseArea {
        width: borderWidth
        height: borderWidth
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        cursorShape: Qt.SizeFDiagCursor
        hoverEnabled: true
        onPressed: targetWindow.controller.startSystemResize(targetWindow, Qt.RightEdge | Qt.BottomEdge)
    }
}

