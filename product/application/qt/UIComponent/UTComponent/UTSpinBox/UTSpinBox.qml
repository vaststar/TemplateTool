import QtQuick
import QtQuick.Controls
import UIComponentBase 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

/**
 * UTSpinBox - Themed spin box with UT design tokens.
 *
 * Features:
 * - Themed background, text, and +/- buttons
 * - Rounded border matching other UT input controls
 * - Smooth hover/press color transitions
 * - Focus ring via UTFocusItem
 *
 * Usage:
 *   UTSpinBox {
 *       from: 0; to: 100; value: 4
 *       onValueChanged: myModel.count = value
 *   }
 */
BaseSpinBox {
    id: control

    // === Configurable properties ===
    property real fieldHeight: 32
    property real buttonWidth: 28
    property int borderRadius: 4

    property var backgroundColorEnum: UIColorToken.Spinbox_Background
    property var textColorEnum: UIColorToken.Spinbox_Text
    property var buttonColorEnum: UIColorToken.Spinbox_Button
    property var borderColorEnum: UIColorToken.Content_Input_Border

    implicitWidth: 120
    implicitHeight: fieldHeight

    font: UTComponentUtil.getUIFont(UIFontToken.Body_Text)

    contentItem: TextInput {
        z: 2
        text: control.textFromValue(control.value, control.locale)
        anchors.fill: parent
        anchors.leftMargin: control.buttonWidth + 4
        anchors.rightMargin: control.buttonWidth + 4
        font: control.font
        color: UTComponentUtil.getPlainUIColor(
                   control.textColorEnum,
                   control.enabled ? UIColorState.Normal : UIColorState.Disabled)
        selectionColor: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
        selectedTextColor: UTComponentUtil.getPlainUIColor(control.textColorEnum, UIColorState.Normal)
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhDigitsOnly

        onTextEdited: {
            var newVal = control.valueFromText(text, control.locale)
            if (newVal >= control.from && newVal <= control.to)
                control.value = newVal
        }
    }

    up.indicator: Rectangle {
        x: parent.width - width
        height: parent.height
        width: control.buttonWidth
        radius: control.borderRadius
        color: {
            if (!control.enabled)
                return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Disabled)
            if (control.up.pressed)
                return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Pressed)
            if (control.up.hovered)
                return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Hovered)
            return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Normal)
        }
        Behavior on color { ColorAnimation { duration: 100 } }

        Text {
            anchors.centerIn: parent
            text: "+"
            font.pixelSize: 14
            font.bold: true
            color: UTComponentUtil.getPlainUIColor(
                       control.textColorEnum,
                       control.enabled ? UIColorState.Normal : UIColorState.Disabled)
        }
    }

    down.indicator: Rectangle {
        x: 0
        height: parent.height
        width: control.buttonWidth
        radius: control.borderRadius
        color: {
            if (!control.enabled)
                return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Disabled)
            if (control.down.pressed)
                return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Pressed)
            if (control.down.hovered)
                return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Hovered)
            return UTComponentUtil.getPlainUIColor(control.buttonColorEnum, UIColorState.Normal)
        }
        Behavior on color { ColorAnimation { duration: 100 } }

        Text {
            anchors.centerIn: parent
            text: "\u2212"
            font.pixelSize: 14
            font.bold: true
            color: UTComponentUtil.getPlainUIColor(
                       control.textColorEnum,
                       control.enabled ? UIColorState.Normal : UIColorState.Disabled)
        }
    }

    background: Rectangle {
        radius: control.borderRadius
        color: UTComponentUtil.getPlainUIColor(
                   control.backgroundColorEnum,
                   control.enabled ? UIColorState.Normal : UIColorState.Disabled)
    }

    // Border drawn ABOVE the +/- buttons. The background's own border would be
    // occluded on the left/right by the opaque button rectangles, leaving only
    // the centre segment visible around the edit — which reads as a second,
    // inner focus frame. Painting the border here keeps it a single clean
    // rounded rect around the whole control in every state.
    Rectangle {
        z: 5
        anchors.fill: parent
        enabled: false
        radius: control.borderRadius
        color: "transparent"
        border.width: 1
        border.color: UTComponentUtil.getPlainUIColor(
                          control.borderColorEnum,
                          control.activeFocus ? UIColorState.Focused : UIColorState.Normal)
        Behavior on border.color { ColorAnimation { duration: 100 } }
    }

    UTFocusItem {
        focusRadius: control.borderRadius
    }
}
