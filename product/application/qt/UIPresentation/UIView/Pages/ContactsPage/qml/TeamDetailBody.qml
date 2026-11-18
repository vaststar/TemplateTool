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
        text: qsTr("Team information")
        fontEnum: UIFontToken.Body_Text_Medium
        colorEnum: UIColorToken.Content_Text
        Layout.bottomMargin: 4
    }
    DetailFieldRow {
        label: qsTr("Team lead")
        value: root.team
            ? (root.team.teamLeadName && root.team.teamLeadName.length > 0
               ? qsTr("%1 (%2)").arg(root.team.teamLeadName).arg(root.team.teamLeadId)
               : root.team.teamLeadId)
            : ""
    }
    DetailFieldRow {
        label: qsTr("Mission")
        value: root.team ? (root.team.mission || "") : ""
    }
}
