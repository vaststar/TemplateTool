import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import UTComponent 1.0
import UIResourceLoader 1.0
/**
 * MessageDialog - Generic message dialog used by UIViewHelper::showMessageAsync.
 *
 * Properties are injected at construction time by UIViewHelper via
 * QQmlComponent::createWithInitialProperties().
 *
 * QML -> C++ contract:
 *   signal accepted(int index)
 *     - emitted exactly once per dialog lifetime.
 *     - `index` is the 0-based index into `buttonsModel`, or -1 if the dialog
 *       was closed externally and no isCancel button was defined.
 *
 * Keyboard handling:
 *   - Enter / Return -> button with isDefault === true (if any)
 *   - Escape         -> button with isCancel  === true (if any), else accepted(-1)
 *   - Window close   -> same as Escape
 */

UTDialog {
    id: dialog

    property string titleText: ""
    property string messageText: ""
    property string detailText: ""
    property int iconKind: 0
    property var buttonsModel: []

    readonly property var _iconTokenMap: ({
        1: UIAssetImageToken.Msg_Info,
        2: UIAssetImageToken.Msg_Warning,
        3: UIAssetImageToken.Msg_Error,
        4: UIAssetImageToken.Msg_Question,
        5: UIAssetImageToken.Msg_Success,
    })

    readonly property var _iconColorMap: ({
        1: "#2196F3",
        2: "#FB8C00",
        3: "#E53935",
        4: "#7E57C2",
        5: "#43A047",
    })

    signal accepted(int index)

    title: titleText
    width: 460
    height: contentColumn.implicitHeight + 48
    minimumWidth: 360
    minimumHeight: 180

    readonly property int cancelIndex: {
        for (var i = 0; i < buttonsModel.length; ++i) {
            if (buttonsModel[i].isCancel === true) return i
        }
        return -1
    }

    property bool _emitted: false

    function _emit(idx) {
        if (dialog._emitted) return
        dialog._emitted = true
        dialog.accepted(idx)
        dialog.close()
    }

    onClosing: function(close) {
        if (!dialog._emitted) {
            dialog._emitted = true
            dialog.accepted(dialog.cancelIndex)
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: dialog._emit(dialog.cancelIndex)
    }

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        UTText {
            visible: messageText.length > 0
            text: messageText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Qt.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            Item {
                visible: dialog.iconKind !== 0 && iconImage.source.toString().length > 0
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                Layout.alignment: Qt.AlignLeft

                Image {
                    id: iconImage
                    anchors.fill: parent
                    source: {
                        var token = dialog._iconTokenMap[dialog.iconKind]
                        return token ? UTComponentUtil.getImageResourcePath(token) : ""
                    }
                    sourceSize.width: 32
                    sourceSize.height: 32
                    fillMode: Image.PreserveAspectFit
                    visible: false
                    layer.enabled: true
                }

                MultiEffect {
                    anchors.fill: parent
                    source: iconImage
                    brightness: 1.0
                    colorization: 1.0
                    colorizationColor: dialog._iconColorMap[dialog.iconKind] || "#000000"
                }
            }

            UTText {
                text: detailText
                wrapMode: Text.WordWrap
                opacity: 0.75
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 12

            Repeater {
                model: buttonsModel

                UTButton {
                    required property var modelData
                    required property int index

                    text: modelData.text
                    enabled: modelData.enabled
                    ToolTip.text: modelData.tooltip
                    ToolTip.visible: hovered && modelData.tooltip.length > 0

                    Keys.onReturnPressed: if (modelData.isDefault) dialog._emit(index)
                    Keys.onEnterPressed: if (modelData.isDefault) dialog._emit(index)

                    onClicked: dialog._emit(index)

                    Component.onCompleted: {
                        if (modelData.isDefault) {
                            forceActiveFocus()
                        }
                    }
                }
            }
        }
    }
}
