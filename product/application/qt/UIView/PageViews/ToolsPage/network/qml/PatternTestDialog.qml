import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Dialog {
    id: root

    required property NetworkProxyController controller

    // ── Theme ──
    readonly property color _sectionBg:      UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder:   UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border,     UIColorState.Normal)
    readonly property color _sectionTitle:    UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Title,      UIColorState.Normal)
    readonly property color _headerBg:        Qt.darker(_sectionBg, 1.08)
    readonly property color _inputBg:         UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background,   UIColorState.Normal)
    readonly property color _inputText:       UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text,         UIColorState.Normal)
    readonly property color _inputBorder:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Normal)
    readonly property color _inputPlaceholder:UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Placeholder,  UIColorState.Normal)
    readonly property color _accentColor:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Focused)
    readonly property font  _inputFont:       UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    readonly property font  _monoFont:        Qt.font({family: "Consolas", pixelSize: _inputFont.pixelSize})

    component ThemedInput : TextField {
        font: root._monoFont; color: root._inputText
        placeholderTextColor: root._inputPlaceholder
        background: Rectangle { color: root._inputBg; border.color: parent.activeFocus ? root._accentColor : root._inputBorder; border.width: 1; radius: 4 }
    }

    function openWithPattern(pattern) {
        ptPattern.text = pattern
        ptResult.text = ""
        open()
    }

    title: qsTr("Test URL Pattern")
    anchors.centerIn: parent; width: 520; implicitHeight: ptDialogCol.implicitHeight + 100
    modal: true; standardButtons: Dialog.Close
    background: Rectangle { color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 8 }
    header: Rectangle {
        width: parent.width; height: 40; color: root._headerBg; radius: 8
        UTText { anchors.centerIn: parent; text: qsTr("🧪 Regex Pattern Tester"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
    }

    ColumnLayout {
        id: ptDialogCol
        width: parent.width; spacing: 12
        GridLayout {
            columns: 2; columnSpacing: 10; rowSpacing: 8; Layout.fillWidth: true
            UTText { text: qsTr("Pattern:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
            ThemedInput { id: ptPattern; Layout.fillWidth: true; placeholderText: qsTr("Regex pattern (e.g. /api/user.*)") }
            UTText { text: qsTr("Test URL:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
            ThemedInput { id: ptTestUrl; Layout.fillWidth: true; placeholderText: qsTr("https://example.com/api/user/123") }
        }
        RowLayout {
            spacing: 8; Layout.alignment: Qt.AlignHCenter
            UTButton {
                text: qsTr("▶ Run Test")
                enabled: ptPattern.text.length > 0 && ptTestUrl.text.length > 0
                onClicked: ptResult.text = root.controller.rulesManager.testUrlPattern(ptPattern.text, ptTestUrl.text)
            }
            UTButton {
                text: qsTr("Clear")
                onClicked: { ptResult.text = ""; ptTestUrl.text = "" }
            }
        }
        Rectangle {
            Layout.fillWidth: true; implicitHeight: ptResult.implicitHeight + 16; radius: 4
            visible: ptResult.text.length > 0
            color: ptResult.text.indexOf("✓") >= 0 ? Qt.alpha("#4CAF50", 0.1) : Qt.alpha("#F44336", 0.1)
            border.color: ptResult.text.indexOf("✓") >= 0 ? Qt.alpha("#4CAF50", 0.3) : Qt.alpha("#F44336", 0.3)
            border.width: 1
            UTText {
                id: ptResult; anchors.centerIn: parent
                fontEnum: UIFontToken.Body_Text; font.family: "Consolas"
                color: text.indexOf("✓") >= 0 ? "#4CAF50" : "#F44336"
                wrapMode: Text.WordWrap; width: parent.width - 16
                horizontalAlignment: Text.AlignHCenter
            }
        }
        UTText {
            text: qsTr("Tip: Pattern uses Python-style regex (re.search). It matches anywhere in the URL, not just from the start.")
            fontEnum: UIFontToken.Caption_Text; colorEnum: UIColorToken.Content_Text; opacity: 0.5
            wrapMode: Text.WordWrap; Layout.fillWidth: true
        }
    }
}
