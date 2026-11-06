import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Rectangle
{
    id: root

    required property ApplicationWindow appWindow

    property MainWindowTitleBarController controller: MainWindowTitleBarController {}

    signal avatarClicked()

    readonly property bool fullScreen: appWindow.visibility === Window.FullScreen
    readonly property bool expandedClientArea:
        (appWindow.flags & Qt.ExpandedClientAreaHint) !== 0

    readonly property real topInset: fullScreen ? appWindow.SafeArea.margins.top : 0

    readonly property real captionHeight: fullScreen || !expandedClientArea
        ? 40 : Math.max(40, appWindow.SafeArea.margins.top)

    readonly property real leftReservedWidth:
        Qt.platform.os === "osx" ? 72 : 0

    readonly property real rightReservedWidth:
    {
        if (Qt.platform.os === "windows")
        {
            return Math.max(160,
                Math.ceil(Math.max(32, root.appWindow.SafeArea.margins.top) * 1.5 * 3) + 8)
        }

        return 0
    }

    readonly property real horizontalPadding: 8
    readonly property real contentSpacing: 8
    readonly property real avatarSize: 32
    readonly property real titleSideMargin: Math.max(
        leftReservedWidth + horizontalPadding + avatarSize + contentSpacing,
        rightReservedWidth + horizontalPadding)

    readonly property bool windowGesturesEnabled:
        !fullScreen && expandedClientArea

    implicitHeight: topInset + captionHeight

    color: UTComponentUtil.getPlainUIColor(UIColorToken.Titlebar_Background, UIColorState.Normal)
    visible: controller.visible

    UTText
    {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: root.topInset

        width: Math.max(0, root.width - 2 * root.titleSideMargin)
        height: root.captionHeight

        text: root.controller.title
        textFormat: Text.PlainText
        maximumLineCount: 1
        elide: Text.ElideRight

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        fontEnum: UIFontToken.Body_Text
        colorEnum: UIColorToken.Titlebar_Button_Text
    }

    RowLayout
    {
        anchors.fill: parent
        anchors.topMargin: root.topInset
        anchors.leftMargin: root.leftReservedWidth + root.horizontalPadding
        anchors.rightMargin: root.rightReservedWidth + root.horizontalPadding

        spacing: root.contentSpacing
        layoutDirection: Qt.LeftToRight

        LayoutMirroring.enabled: false
        LayoutMirroring.childrenInherit: true

        UTImageButton
        {
            Layout.preferredWidth: root.avatarSize
            Layout.preferredHeight: root.avatarSize
            Layout.alignment: Qt.AlignVCenter

            source: UTComponentUtil.getImageResourcePath(UIAssetImageToken.Icon_User)
            onClicked: root.avatarClicked()
        }

        Item
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            DragHandler
            {
                target: null
                acceptedButtons: Qt.LeftButton
                enabled: root.windowGesturesEnabled

                onActiveChanged:
                {
                    if (active)
                    {
                        root.appWindow.startSystemMove()
                    }
                }
            }

            TapHandler
            {
                acceptedButtons: Qt.LeftButton
                gesturePolicy: TapHandler.DragThreshold
                enabled: root.windowGesturesEnabled

                onDoubleTapped:
                {
                    if (root.appWindow.visibility === Window.Maximized)
                    {
                        root.appWindow.showNormal()
                    }
                    else
                    {
                        root.appWindow.showMaximized()
                    }
                }
            }
        }
    }
}
