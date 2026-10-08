import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root

    required property NetworkProxyController controller
    signal testPattern(string pattern)

    property string editingId: ""
    property string localError: ""
    property bool showManagerError: false
    readonly property var headerOperations: ["set", "add", "remove"]
    readonly property var bodyOperations: ["none", "replace", "json_set", "json_remove", "text_replace"]
    readonly property var methods: ["ANY", "GET", "POST", "PUT", "PATCH", "DELETE", "HEAD", "OPTIONS", "CONNECT", "TRACE"]
    readonly property string bodyOperation: bodyOperations[bodyType.currentIndex] || "none"
    readonly property color _inputBg: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Background, UIColorState.Normal)
    readonly property color _inputBorder: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Normal)
    readonly property color _accentColor: UTComponentUtil.getPlainUIColor(UIColorToken.Content_Input_Border, UIColorState.Focused)
    readonly property string errorText: localError.length > 0 ? localError
        : showManagerError ? controller.rulesManager.rewriteError : ""

    // Draft rows are local to this form; saved rules always come from C++.
    ListModel { id: headerFormModel }

    function indexForId(ruleId) {
        const rules = root.controller.rulesManager.rewriteRules
        for (let i = 0; i < rules.length; ++i) {
            if (rules[i].id === ruleId)
                return i
        }
        return -1
    }

    function resetForm() {
        root.editingId = ""
        root.localError = ""
        root.showManagerError = false
        ruleUrl.text = ""
        ruleMethod.currentIndex = 0
        ruleStage.currentIndex = 1
        ruleEnabled.checked = true
        headerFormModel.clear()
        bodyType.currentIndex = 0
        bodyValue.text = ""
        jsonPath.text = ""
        findText.text = ""
        replacementText.text = ""
    }

    function editRule(rule) {
        resetForm()
        root.editingId = rule.id
        ruleUrl.text = rule.url_pattern
        ruleMethod.currentIndex = Math.max(0, root.methods.indexOf(rule.method))
        ruleStage.currentIndex = rule.stage === "request" ? 0 : 1
        ruleEnabled.checked = rule.enabled
        const headers = rule.headers || []
        for (let i = 0; i < headers.length; ++i) {
            headerFormModel.append({
                operation: headers[i].operation,
                headerName: headers[i].name,
                headerValue: headers[i].value || ""
            })
        }
        const body = rule.body || { operation: "none" }
        bodyType.currentIndex = Math.max(0, root.bodyOperations.indexOf(body.operation))
        bodyValue.text = body.operation === "json_set" ? JSON.stringify(body.value, null, 2)
            : body.operation === "replace" ? body.value : ""
        jsonPath.text = body.path || ""
        findText.text = body.find || ""
        replacementText.text = body.replacement || ""
        formScroll.contentItem.contentY = 0
    }

    function saveForm() {
        root.localError = ""
        root.showManagerError = true
        const headers = []
        for (let i = 0; i < headerFormModel.count; ++i) {
            const row = headerFormModel.get(i)
            headers.push({ operation: row.operation, name: row.headerName, value: row.headerValue })
        }

        const body = { operation: root.bodyOperation }
        if (body.operation === "replace") {
            body.value = bodyValue.text
        } else if (body.operation === "json_set") {
            body.path = jsonPath.text
            try {
                body.value = JSON.parse(bodyValue.text, function(key, value) {
                    if (typeof value === "number" && !isFinite(value))
                        throw new Error(String(value))
                    return value
                })
            } catch (error) {
                root.localError = qsTr("JSON value is invalid: %1").arg(error.message)
                return
            }
        } else if (body.operation === "json_remove") {
            body.path = jsonPath.text
        } else if (body.operation === "text_replace") {
            body.find = findText.text
            body.replacement = replacementText.text
        }

        const editingIndex = root.editingId.length > 0 ? indexForId(root.editingId) : -1
        if (root.editingId.length > 0 && editingIndex < 0) {
            root.localError = qsTr("This rule was removed. Start a new rule.")
            return
        }
        const rule = {
            enabled: ruleEnabled.checked,
            url_pattern: ruleUrl.text,
            method: ruleMethod.currentText,
            stage: ruleStage.currentIndex === 0 ? "request" : "response",
            headers: headers,
            body: body
        }
        if (root.controller.rulesManager.saveRewriteRule(editingIndex, rule))
            resetForm()
    }

    function bodyLabel(operation) {
        switch (operation) {
        case "replace": return qsTr("Replace body")
        case "json_set": return qsTr("Set JSON field")
        case "json_remove": return qsTr("Remove JSON field")
        case "text_replace": return qsTr("Find and replace text")
        default: return qsTr("Keep body")
        }
    }

    function ruleSummary(rule) {
        const parts = []
        const headerCount = (rule.headers || []).length
        if (headerCount > 0)
            parts.push(qsTr("%1 header changes").arg(headerCount))
        const body = rule.body || { operation: "none" }
        if (body.operation !== "none")
            parts.push(bodyLabel(body.operation))
        return parts.join(" · ")
    }

    Connections {
        target: root.controller.rulesManager
        function onRewriteRulesChanged() {
            if (root.editingId.length > 0 && root.indexForId(root.editingId) < 0)
                root.resetForm()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        UTText {
            text: qsTr("Rewrite Rules")
            fontEnum: UIFontToken.Body_Text_Medium
            colorEnum: UIColorToken.Content_Section_Title
        }
        UTText {
            Layout.fillWidth: true
            text: qsTr("Modify matching requests or responses. Rules run from top to bottom; fields you do not specify stay unchanged.")
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Text
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            UTText {
                text: qsTr("Saved Rules (%1/128)").arg(ruleList.count)
                fontEnum: UIFontToken.Body_Text_Medium
                colorEnum: UIColorToken.Content_Section_Title
            }
            Item { Layout.fillWidth: true }
            UTButton { text: qsTr("New Rule"); onClicked: root.resetForm() }
            UTButton {
                text: qsTr("Clear All")
                enabled: ruleList.count > 0
                onClicked: {
                    root.showManagerError = true
                    root.localError = ""
                    root.controller.rulesManager.clearRewriteRules()
                    root.resetForm()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(160, Math.max(72, ruleList.count * 72 + 8))
            Layout.maximumHeight: Math.max(72, root.height * 0.28)
            color: root._inputBg
            border.color: root._inputBorder
            border.width: 1
            radius: 4

            ListView {
                id: ruleList
                anchors.fill: parent
                anchors.margins: 4
                clip: true
                spacing: 2
                model: root.controller.rulesManager.rewriteRules
                delegate: Rectangle {
                    id: ruleRow
                    required property int index
                    required property var modelData
                    width: ruleList.width
                    height: 68
                    radius: 3
                    color: root.editingId === modelData.id ? Qt.alpha(root._accentColor, 0.12) : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 8
                        UTCheckBox {
                            id: savedEnabledCheck
                            checked: ruleRow.modelData.enabled
                            onToggled: {
                                const ruleId = ruleRow.modelData.id
                                const newEnabled = checked
                                root.localError = ""
                                root.showManagerError = true
                                root.controller.rulesManager.setRewriteRuleEnabled(ruleRow.index, newEnabled)
                                const savedIndex = root.indexForId(ruleId)
                                if (savedIndex >= 0) {
                                    const actualEnabled = root.controller.rulesManager.rewriteRules[savedIndex].enabled
                                    savedEnabledCheck.checked = actualEnabled
                                    if (root.editingId === ruleId)
                                        ruleEnabled.checked = actualEnabled
                                }
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            UTText {
                                Layout.fillWidth: true
                                text: qsTr("%1 · %2 · %3").arg(ruleRow.modelData.stage === "request" ? qsTr("Request") : qsTr("Response"))
                                    .arg(ruleRow.modelData.method).arg(ruleRow.modelData.url_pattern)
                                fontEnum: UIFontToken.Monospace_Text
                                colorEnum: UIColorToken.Content_Text
                                elide: Text.ElideRight
                            }
                            UTText {
                                Layout.fillWidth: true
                                text: root.ruleSummary(ruleRow.modelData)
                                fontEnum: UIFontToken.Caption_Text
                                colorEnum: UIColorToken.Content_Secondary_Text
                                elide: Text.ElideRight
                            }
                        }
                        UTButton { text: qsTr("Edit"); implicitHeight: 26; onClicked: root.editRule(ruleRow.modelData) }
                        UTButton {
                            text: "↑"; implicitWidth: 28; implicitHeight: 26
                            enabled: ruleRow.index > 0
                            onClicked: {
                                root.localError = ""
                                root.showManagerError = true
                                root.controller.rulesManager.moveRewriteRule(ruleRow.index, ruleRow.index - 1)
                            }
                        }
                        UTButton {
                            text: "↓"; implicitWidth: 28; implicitHeight: 26
                            enabled: ruleRow.index < ruleList.count - 1
                            onClicked: {
                                root.localError = ""
                                root.showManagerError = true
                                root.controller.rulesManager.moveRewriteRule(ruleRow.index, ruleRow.index + 1)
                            }
                        }
                        UTButton {
                            text: "✕"; implicitWidth: 28; implicitHeight: 26
                            onClicked: {
                                root.localError = ""
                                root.showManagerError = true
                                root.controller.rulesManager.removeRewriteRule(ruleRow.index)
                            }
                        }
                    }
                }
                UTText {
                    anchors.centerIn: parent
                    visible: ruleList.count === 0
                    text: qsTr("No rewrite rules. Configure one below.")
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                }
                ScrollBar.vertical: ScrollBar { }
            }
        }

        UTScrollView {
            id: formScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 120

            ColumnLayout {
                width: formScroll.availableWidth
                spacing: 12

                UTText {
                    text: root.editingId.length > 0 ? qsTr("Edit Rewrite Rule") : qsTr("New Rewrite Rule")
                    fontEnum: UIFontToken.Body_Text_Medium
                    colorEnum: UIColorToken.Content_Section_Title
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 10
                    rowSpacing: 8
                    UTText { text: qsTr("URL Pattern:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        UTTextField {
                            id: ruleUrl
                            Layout.fillWidth: true
                            fontEnum: UIFontToken.Monospace_Text
                            placeholderText: qsTr("e.g. /api/user.*")
                        }
                        UTButton {
                            text: qsTr("Test")
                            enabled: ruleUrl.text.length > 0
                            onClicked: root.testPattern(ruleUrl.text)
                        }
                    }
                    UTText { text: qsTr("Target:"); fontEnum: UIFontToken.Body_Text; colorEnum: UIColorToken.Content_Text }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        UTComboBox {
                            id: ruleStage
                            Layout.preferredWidth: 120
                            model: [qsTr("Request"), qsTr("Response")]
                            currentIndex: 1
                        }
                        UTComboBox {
                            id: ruleMethod
                            Layout.preferredWidth: 100
                            model: root.methods
                        }
                        UTCheckBox { id: ruleEnabled; text: qsTr("Enabled"); checked: true }
                    }
                }

                UTText { text: qsTr("Header Changes"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                UTText {
                    Layout.fillWidth: true
                    text: qsTr("Set replaces existing values; Add keeps them; Remove deletes the header. Header names are case-insensitive.")
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                    wrapMode: Text.WordWrap
                }
                Repeater {
                    model: headerFormModel
                    delegate: RowLayout {
                        id: headerRow
                        required property int index
                        required property string operation
                        required property string headerName
                        required property string headerValue
                        Layout.fillWidth: true
                        spacing: 8
                        UTComboBox {
                            Layout.preferredWidth: 100
                            model: [qsTr("Set"), qsTr("Add"), qsTr("Remove")]
                            currentIndex: Math.max(0, root.headerOperations.indexOf(headerRow.operation))
                            onActivated: function(selectedIndex) {
                                headerFormModel.setProperty(headerRow.index, "operation", root.headerOperations[selectedIndex])
                            }
                        }
                        UTTextField {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 150
                            fontEnum: UIFontToken.Monospace_Text
                            placeholderText: qsTr("Header name")
                            text: headerRow.headerName
                            onTextEdited: headerFormModel.setProperty(headerRow.index, "headerName", text)
                        }
                        UTTextField {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 200
                            enabled: headerRow.operation !== "remove"
                            fontEnum: UIFontToken.Monospace_Text
                            placeholderText: qsTr("Header value")
                            text: headerRow.headerValue
                            onTextEdited: headerFormModel.setProperty(headerRow.index, "headerValue", text)
                        }
                        UTButton { text: "✕"; implicitWidth: 28; onClicked: headerFormModel.remove(headerRow.index) }
                    }
                }
                UTButton {
                    text: qsTr("+ Header Change")
                    Layout.alignment: Qt.AlignLeft
                    onClicked: headerFormModel.append({ operation: "set", headerName: "", headerValue: "" })
                }

                UTText { text: qsTr("Body Change"); fontEnum: UIFontToken.Body_Text_Medium; colorEnum: UIColorToken.Content_Section_Title }
                UTComboBox {
                    id: bodyType
                    Layout.fillWidth: true
                    model: [qsTr("Keep body"), qsTr("Replace body"), qsTr("Set JSON field"), qsTr("Remove JSON field"), qsTr("Find and replace text")]
                }
                UTTextField {
                    id: jsonPath
                    Layout.fillWidth: true
                    visible: root.bodyOperation === "json_set" || root.bodyOperation === "json_remove"
                    fontEnum: UIFontToken.Monospace_Text
                    placeholderText: qsTr("JSON field path, e.g. /data/enabled")
                }
                UTText {
                    Layout.fillWidth: true
                    visible: jsonPath.visible
                    text: qsTr("Use /items/0 for an array item, /items/- to append, ~0 for ~ and ~1 for /. The parent must exist. An empty path replaces the whole JSON document when setting a value.")
                    fontEnum: UIFontToken.Caption_Text
                    colorEnum: UIColorToken.Content_Secondary_Text
                    wrapMode: Text.WordWrap
                }
                UTText {
                    visible: root.bodyOperation === "replace" || root.bodyOperation === "json_set"
                    text: root.bodyOperation === "json_set" ? qsTr("JSON Value:") : qsTr("Replacement Body:")
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Text
                }
                UTScrollView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 130
                    visible: root.bodyOperation === "replace" || root.bodyOperation === "json_set"
                    UTTextArea {
                        id: bodyValue
                        wrapMode: TextEdit.Wrap
                        placeholderText: root.bodyOperation === "json_set"
                            ? qsTr("Enter valid JSON: true, null, 123, \"text\", an array or an object")
                            : qsTr("Enter the new body. Leave empty to send an empty body.")
                    }
                }
                UTText {
                    visible: root.bodyOperation === "text_replace"
                    text: qsTr("Find Text (literal):")
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Text
                }
                UTScrollView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 90
                    visible: root.bodyOperation === "text_replace"
                    UTTextArea { id: findText; wrapMode: TextEdit.Wrap; placeholderText: qsTr("Text to find") }
                }
                UTText {
                    visible: root.bodyOperation === "text_replace"
                    text: qsTr("Replace With:")
                    fontEnum: UIFontToken.Body_Text
                    colorEnum: UIColorToken.Content_Text
                }
                UTScrollView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 90
                    visible: root.bodyOperation === "text_replace"
                    UTTextArea { id: replacementText; wrapMode: TextEdit.Wrap; placeholderText: qsTr("Replacement text; leave empty to remove matches") }
                }
                Item { Layout.preferredHeight: 4 }
            }
        }

        UTText {
            Layout.fillWidth: true
            visible: root.errorText.length > 0
            text: root.errorText
            fontEnum: UIFontToken.Caption_Text
            colorEnum: UIColorToken.Content_Error_Text
            wrapMode: Text.WordWrap
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            UTButton {
                text: root.editingId.length > 0 ? qsTr("Save Changes") : qsTr("Add Rule")
                enabled: root.editingId.length > 0 || ruleList.count < 128
                onClicked: root.saveForm()
            }
            UTButton { text: qsTr("Reset Form"); onClicked: root.resetForm() }
            Item { Layout.fillWidth: true }
        }
    }
}
