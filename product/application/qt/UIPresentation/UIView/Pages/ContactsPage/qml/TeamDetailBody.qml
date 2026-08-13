import QtQuick
import QtQuick.Layouts
import UTComponent 1.0
import UIResourceLoader 1.0

// Team-specific section. Bound to info.team (teamLeadId / teamLeadName / mission).
ColumnLayout {
    id: root
    property var team

    spacing: 8
    visible: !!root.team

    UTText {
        text: "团队信息"
        fontEnum: UIFontToken.Body_Text_Medium
        colorEnum: UIColorToken.Content_Text
        Layout.bottomMargin: 4
    }
    DetailFieldRow {
        label: "负责人"
        value: root.team
            ? (root.team.teamLeadName && root.team.teamLeadName.length > 0
               ? root.team.teamLeadName + "（" + root.team.teamLeadId + "）"
               : root.team.teamLeadId)
            : ""
    }
    DetailFieldRow {
        label: "目标"
        value: root.team ? (root.team.mission || "") : ""
    }
}
