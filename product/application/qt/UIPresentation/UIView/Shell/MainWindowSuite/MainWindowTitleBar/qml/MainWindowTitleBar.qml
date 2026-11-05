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

    property int systemButtonsEdge: Qt.RightEdge
    property real systemButtonsWidth: 0
    property real captionHeight: 40
    property bool showTitle: false

    signal avatarClicked()

    readonly property bool fullScreen: appWindow.visibility === Window.FullScreen
    readonly property real topInset: fullScreen ? appWindow.SafeArea.margins.top : 0

    readonly property real leftReservedWidth:
        systemButtonsEdge === Qt.LeftEdge ? systemButtonsWidth : 0

    readonly property real rightReservedWidth:
        systemButtonsEdge === Qt.RightEdge ? systemButtonsWidth : 0

    readonly property bool windowGesturesEnabled:
        !fullScreen && (appWindow.flags & Qt.ExpandedClientAreaHint) !== 0

    implicitHeight: topInset + Math.max(40, captionHeight)

    color: UTComponentUtil.getPlainUIColor(UIColorToken.Titlebar_Background, UIColorState.Normal)
    visible: controller.visible

    RowLayout
    {
        anchors.fill: parent
        anchors.topMargin: root.topInset
        anchors.leftMargin: root.leftReservedWidth + 8
        anchors.rightMargin: root.rightReservedWidth + 8

        spacing: 8
        layoutDirection: Qt.LeftToRight

        LayoutMirroring.enabled: false
        LayoutMirroring.childrenInherit: true

        UTImageButton
        {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: Math.max(0, (root.captionHeight - 24) / 2)

            source: UTComponentUtil.getImageResourcePath(UIAssetImageToken.Icon_User)
            onClicked: root.avatarClicked()
        }

        Item
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            UTText
            {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top

                height: root.captionHeight
                visible: root.showTitle

                text: root.appWindow.title
                textFormat: Text.PlainText
                maximumLineCount: 1
                elide: Text.ElideRight

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                fontEnum: UIFontToken.Body_Text
                colorEnum: UIColorToken.Titlebar_Button_Text
            }

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
