import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Dialog {
    id: root

    // ── Theme ──
    readonly property color _sectionBg:      UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Background, UIColorState.Normal)
    readonly property color _sectionBorder:   UTComponentUtil.getPlainUIColor(UIColorToken.Content_Section_Border,     UIColorState.Normal)
    readonly property color _headerBg:        Qt.darker(_sectionBg, 1.08)
    readonly property color _inputBg:         UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background,   UIColorState.Normal)
    readonly property color _inputText:       UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Text,         UIColorState.Normal)
    readonly property color _inputBorder:     UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border,       UIColorState.Normal)
    readonly property font  _inputFont:       UTComponentUtil.getUIFont(UIFontToken.Body_Text)
    readonly property font  _monoFont:        UTComponentUtil.getUIFont(UIFontToken.Monospace_Text)

    property var ruleData: ({})

    signal editRule(var data)
    signal testRulePattern(string pattern)

    function openWithRule(data) {
        ruleData = data
        open()
    }

    title: qsTr("Mock Rule Detail")
    anchors.centerIn: parent; width: 540; implicitHeight: mdDialogCol.implicitHeight + 100
    modal: true; standardButtons: Dialog.Close
    background: Rectangle { color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 8 }
    header: Rectangle {
        width: parent.width; height: 40; color: root._headerBg; radius: 8
        UTText { anchors.centerIn: parent; text: qsTr("🎭 Mock Rule Detail"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
    }

    ColumnLayout {
        id: mdDialogCol
        width: parent.width; spacing: 8
        GridLayout {
            columns: 2; columnSpacing: 10; rowSpacing: 6; Layout.fillWidth: true
            UTText { text: qsTr("URL Pattern:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
            UTText { text: root.ruleData.url_pattern || ""; fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
            UTText { text: qsTr("Status Code:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
            UTText { text: String(root.ruleData.status_code || 200); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
            UTText { text: qsTr("Content-Type:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
            UTText { text: root.ruleData.content_type || "application/json"; fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
            UTText { text: qsTr("Headers:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
            UTText { text: root.ruleData.headers || "(none)"; fontEnum: UIFontToken.Monospace_Text; colorEnum: UIColorToken.Content_Text; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
        }
        UTText { text: qsTr("Response Body:"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Text }
        Rectangle {
            Layout.fillWidth: true; implicitHeight: Math.max(80, mdBody.implicitHeight + 12); radius: 4
            color: root._inputBg; border.color: root._inputBorder; border.width: 1
            UTScrollView {
                anchors.fill: parent
                UTTextArea {
                    id: mdBody; readOnly: true; text: root.ruleData.body || ""
                    padding: 6
                    background: null
                }
            }
        }
        RowLayout {
            spacing: 8
            UTButton {
                text: qsTr("Edit (load into form)")
                onClicked: { root.editRule(root.ruleData); root.close() }
            }
            UTButton {
                text: qsTr("🧪 Test Pattern")
                onClicked: { root.testRulePattern(root.ruleData.url_pattern || ""); root.close() }
            }
        }
    }
}
