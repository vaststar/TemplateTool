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

    function openWithPattern(pattern) {
        ptPattern.text = pattern
        ptResult.text = ""
        open()
    }

    title: qsTr("Test URL Pattern")
    anchors.centerIn: parent; width: 520; implicitHeight: ptDialogCol.implicitHeight + 100
    modal: true
    background: Rectangle { color: root._sectionBg; border.color: root._sectionBorder; border.width: 1; radius: 8 }
    header: Rectangle {
        width: parent.width; height: 40; color: root._headerBg; radius: 8
        UTText { anchors.centerIn: parent; text: qsTr("🧪 Regex Pattern Tester"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
    }
    footer: Item {
        implicitHeight: 48
        UTButton {
            text: qsTr("Close")
            anchors.centerIn: parent
            onClicked: root.close()
        }
    }

    ColumnLayout {
        id: ptDialogCol
        width: parent.width; spacing: 12
        GridLayout {
            columns: 2; columnSpacing: 10; rowSpacing: 8; Layout.fillWidth: true
            UTText { text: qsTr("Pattern:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
            UTTextField { id: ptPattern; Layout.fillWidth: true; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("Regex pattern (e.g. /api/user.*)") }
            UTText { text: qsTr("Test URL:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
            UTTextField { id: ptTestUrl; Layout.fillWidth: true; fontEnum: UIFontToken.Monospace_Text; placeholderText: qsTr("https://example.com/api/user/123") }
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
                fontEnum: UIFontToken.Monospace_Text
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
