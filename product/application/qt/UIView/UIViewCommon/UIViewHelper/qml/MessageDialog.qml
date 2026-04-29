import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
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

    // Injected by C++.
    property string titleText:   ""
    property string messageText: ""
    property string detailText:  ""
    property int    iconKind:    0       // 0=None,1=Info,2=Warning,3=Error,4=Question
    property var    buttonsModel: []     // [{ text, tooltip, role, isDefault, isCancel, enabled }, ...]

    signal accepted(int index)

    title: titleText
    width: 460
    height: contentColumn.implicitHeight + 48
    minimumWidth: 360
    minimumHeight: 180

    // Cancel index resolved once from buttonsModel; -1 means no cancel button.
    readonly property int cancelIndex: {
        for (var i = 0; i < buttonsModel.length; ++i) {
            if (buttonsModel[i].isCancel === true) return i
        }
        return -1
    }

    // Guard: emit accepted() at most once even if both a button click and a
    // window-close event arrive.
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
            visible: dialog.messageText.length > 0
            text: dialog.messageText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        UTText {
            visible: dialog.detailText.length > 0
            text: dialog.detailText
            wrapMode: Text.WordWrap
            opacity: 0.75
            Layout.fillWidth: true
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 12

            Repeater {
                model: dialog.buttonsModel

                UTButton {
                    required property var modelData
                    required property int index

                    text:            modelData.text
                    enabled:         modelData.enabled
                    ToolTip.text:    modelData.tooltip
                    ToolTip.visible: hovered && modelData.tooltip.length > 0

                    Keys.onReturnPressed: if (modelData.isDefault) dialog._emit(index)
                    Keys.onEnterPressed:  if (modelData.isDefault) dialog._emit(index)

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
