import QtQuick
import QtQuick.Layouts
import QtQml.Models
import UTComponent 1.0
import UIResourceLoader 1.0
import "../common"

GalleryCase {
    id: root
    objectName: "utGridViewCase"

    title: "UTGridView"

    readonly property var selectedTemplate: templateGrid.selectedIndex >= 0
                                            ? templateModel.get(templateGrid.selectedIndex) : null

    ListModel {
        id: templateModel
        objectName: "templateGridModel"

        ListElement { templateKey: "brief"; templateName: qsTr("Project brief"); category: qsTr("Planning"); dimensions: "A4 portrait"; pageCount: 3; summary: qsTr("A concise project overview with goals, scope, and milestones.") }
        ListElement { templateKey: "research"; templateName: qsTr("Research plan"); category: qsTr("Research"); dimensions: "A4 portrait"; pageCount: 5; summary: qsTr("Research questions, participant criteria, and a session outline.") }
        ListElement { templateKey: "workshop"; templateName: qsTr("Workshop board"); category: qsTr("Collaboration"); dimensions: "1920 × 1080"; pageCount: 1; summary: qsTr("A shared canvas for workshop prompts, notes, and decisions.") }
        ListElement { templateKey: "roadmap"; templateName: qsTr("Product roadmap"); category: qsTr("Planning"); dimensions: "1920 × 1080"; pageCount: 2; summary: qsTr("A quarterly view of themes, priorities, and delivery milestones.") }
        ListElement { templateKey: "report"; templateName: qsTr("Weekly report"); category: qsTr("Reporting"); dimensions: "A4 portrait"; pageCount: 2; summary: qsTr("Progress, upcoming work, and decisions for a weekly team update.") }
        ListElement { templateKey: "proposal"; templateName: qsTr("Client proposal"); category: qsTr("Business"); dimensions: "A4 portrait"; pageCount: 6; summary: qsTr("An introduction to a proposed engagement, timeline, and deliverables.") }
        ListElement { templateKey: "presentation"; templateName: qsTr("Team presentation"); category: qsTr("Presentation"); dimensions: "1920 × 1080"; pageCount: 12; summary: qsTr("A presentation with space for context, key findings, and next steps.") }
        ListElement { templateKey: "review"; templateName: qsTr("Design review"); category: qsTr("Design"); dimensions: "1920 × 1080"; pageCount: 8; summary: qsTr("A structured walkthrough of design options and review questions.") }
        ListElement { templateKey: "notes"; templateName: qsTr("Meeting notes"); category: qsTr("Collaboration"); dimensions: "A4 portrait"; pageCount: 2; summary: qsTr("An agenda, discussion notes, decisions, and assigned actions.") }
        ListElement { templateKey: "onboarding"; templateName: qsTr("Welcome guide"); category: qsTr("Learning"); dimensions: "A4 portrait"; pageCount: 7; summary: qsTr("A welcome pack covering the team, tools, and first-week activities.") }
        ListElement { templateKey: "campaign"; templateName: qsTr("Campaign plan"); category: qsTr("Marketing"); dimensions: "A4 landscape"; pageCount: 4; summary: qsTr("Audience, messaging, channels, and a coordinated campaign schedule.") }
        ListElement { templateKey: "case-study"; templateName: qsTr("Case study"); category: qsTr("Design"); dimensions: "A4 portrait"; pageCount: 5; summary: qsTr("A project story covering the challenge, process, and outcomes.") }
        ListElement { templateKey: "checklist"; templateName: qsTr("Launch checklist"); category: qsTr("Planning"); dimensions: "A4 portrait"; pageCount: 2; summary: qsTr("Readiness checks and owners for a coordinated product launch.") }
        ListElement { templateKey: "feedback"; templateName: qsTr("Feedback summary"); category: qsTr("Research"); dimensions: "A4 portrait"; pageCount: 4; summary: qsTr("A summary of customer themes, supporting evidence, and opportunities.") }
        ListElement { templateKey: "handbook"; templateName: qsTr("Team handbook"); category: qsTr("Operations"); dimensions: "A4 portrait"; pageCount: 10; summary: qsTr("Working agreements, team practices, and frequently used resources.") }
        ListElement { templateKey: "retrospective"; templateName: qsTr("Retrospective"); category: qsTr("Collaboration"); dimensions: "1920 × 1080"; pageCount: 1; summary: qsTr("A shared board for reflections, learnings, and improvements.") }
    }

    GalleryDemo {
        title: qsTr("Template library")

        UTGridView {
            id: templateGrid
            objectName: "templateGridView"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 600
            Layout.preferredHeight: 184
            model: templateModel
            selectedIndex: 0
            readonly property int columns: Math.max(1, Math.floor(contentAvailableWidth / 150))
            cellWidth: Math.max(1, contentAvailableWidth / columns)
            cellHeight: 88

            onItemInvoked: function(index) {
                selectedIndex = index
            }

            delegate: Item {
                id: templateCell
                objectName: "templateGridDelegate_" + templateKey
                required property int index
                required property string templateKey
                required property string templateName
                required property string category

                width: templateGrid.cellWidth
                height: templateGrid.cellHeight

                UTToolButton {
                    id: templateButton
                    objectName: "templateGridButton_" + templateCell.templateKey
                    anchors.fill: parent
                    anchors.margins: 4
                    focusPolicy: Qt.NoFocus
                    activeFocusOnTab: false
                    text: templateCell.templateName
                    leftPadding: 12
                    rightPadding: 12
                    topPadding: 10
                    bottomPadding: 10
                    backgroundColorEnum: UIColorToken.Listview_Item_Background
                    readonly property bool selected: templateGrid.selectedIndex === templateCell.index

                    contentItem: ColumnLayout {
                        spacing: 6

                        UTLabel {
                            Layout.fillWidth: true
                            text: templateCell.templateName
                            fontEnum: UIFontToken.Body_Text_Medium
                            colorEnum: UIColorToken.Listview_Item_Text
                            colorState: templateButton.selected ? UIColorState.Selected : UIColorState.Normal
                            elide: Text.ElideRight
                        }
                        UTLabel {
                            Layout.fillWidth: true
                            text: templateCell.category
                            fontEnum: UIFontToken.Caption_Text
                            colorEnum: UIColorToken.Content_Secondary_Text
                            elide: Text.ElideRight
                        }
                    }

                    UTFocusItem {
                        objectName: "templateGridFocus_" + templateCell.templateKey
                        delegateFocused: templateGrid.gridView.activeFocus
                                         && templateGrid.currentIndex === templateCell.index
                        focusRadius: templateButton.radius
                    }

                    onClicked: {
                        templateGrid.focusOnItem(templateCell.index)
                        templateGrid.itemInvoked(templateCell.index)
                    }
                }
            }
        }

        ColumnLayout {
            objectName: "templateGridSelectionDetails"
            Layout.fillWidth: true
            spacing: 4

            UTLabel {
                objectName: "templateGridSelectedName"
                Layout.fillWidth: true
                text: root.selectedTemplate ? root.selectedTemplate.templateName : ""
                fontEnum: UIFontToken.Body_Text_Medium
                wrapMode: Text.Wrap
            }
            UTLabel {
                objectName: "templateGridSelectedFormat"
                Layout.fillWidth: true
                text: root.selectedTemplate
                      ? (root.selectedTemplate.pageCount === 1
                         ? qsTr("%1 · 1 page").arg(root.selectedTemplate.dimensions)
                         : qsTr("%1 · %2 pages").arg(root.selectedTemplate.dimensions).arg(root.selectedTemplate.pageCount)) : ""
                colorEnum: UIColorToken.Content_Secondary_Text
                wrapMode: Text.Wrap
            }
            UTLabel {
                objectName: "templateGridSelectedSummary"
                Layout.fillWidth: true
                text: root.selectedTemplate ? root.selectedTemplate.summary : ""
                wrapMode: Text.Wrap
            }
        }
    }
}
