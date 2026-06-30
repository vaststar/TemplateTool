import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import UTComponent 1.0
import UTComposite 1.0

/**
 * UTMessageDialog - Generic message dialog driven by UTMessageDialogController.
 *
 * The dialog owns its controller (default-constructed inline). The C++ side
 * grabs the controller via UIView::UIViewHelper::controllerOf<>() right after
 * the window is created and calls setOptions() exactly once before show().
 *
 * QML -> C++ contract:
 *   - All button clicks, the Esc shortcut, and the window-close all forward
 *     to controller.accept(index). The controller emits its `closed(result)`
 *     signal exactly once per lifetime.
 */
UTDialog {
    id: dialog

    property UTMessageDialogController controller: UTMessageDialogController{}

    title: controller.titleText
    width: 460
    height: contentColumn.implicitHeight + 48
    minimumWidth: 360
    minimumHeight: 180
    visible: false

    onClosing: function(close) {
        controller.accept(controller.cancelIndex)
    }

    Connections {
        target: dialog.controller
        function onClosed(result) { dialog.close() }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: controller.accept(controller.cancelIndex)
    }

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // ----- Top row: icon + (message / detail) -----
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // Icon: visual anchor for the whole text block. Top-aligned so
            // long detail text doesn't drag it to the middle.
            Item {
                visible: controller.iconAssetToken !== 0
                         && iconImage.source.toString().length > 0
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignVCenter

                Image {
                    id: iconImage
                    anchors.fill: parent
                    source: controller.iconAssetToken !== 0
                            ? UTComponentUtil.getImageResourcePath(controller.iconAssetToken)
                            : ""
                    sourceSize.width: 40
                    sourceSize.height: 40
                    fillMode: Image.PreserveAspectFit
                    visible: false
                    layer.enabled: true
                }

                MultiEffect {
                    anchors.fill: parent
                    source: iconImage
                    brightness: 1.0
                    colorization: 1.0
                    colorizationColor: controller.iconColor.length > 0
                                       ? controller.iconColor : "#000000"
                }
            }

            // Text column: message (main instruction) on top, detail below.
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: 8

                UTText {
                    visible: controller.messageText.length > 0
                    text: controller.messageText
                    fontEnum: UIFontToken.Title_Text
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                UTText {
                    visible: controller.detailText.length > 0
                    text: controller.detailText
                    fontEnum: UIFontToken.Body_Text
                    wrapMode: Text.WordWrap
                    opacity: 0.75
                    Layout.fillWidth: true
                }
            }
        }

        // ----- Bottom row: buttons, right-aligned -----
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 12

            Repeater {
                model: controller.buttonsModel

                UTButton {
                    required property var modelData
                    required property int index

                    text: modelData.text
                    enabled: modelData.enabled
                    ToolTip.text: modelData.tooltip
                    ToolTip.visible: hovered && modelData.tooltip.length > 0

                    Keys.onReturnPressed: if (modelData.isDefault) controller.accept(index)
                    Keys.onEnterPressed: if (modelData.isDefault) controller.accept(index)

                    onClicked: controller.accept(index)

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
