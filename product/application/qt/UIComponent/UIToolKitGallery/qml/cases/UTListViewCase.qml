import QtQuick
import QtQuick.Layouts
import QtQml.Models
import UTComponent 1.0
import UIResourceLoader 1.0
import "../common"

GalleryCase {
    id: root
    objectName: "utListViewCase"

    title: "UTListView"

    readonly property var selectedWorkspace: workspaceList.selectedIndex >= 0
                                             ? workspaceModel.get(workspaceList.selectedIndex) : null

    ListModel {
        id: workspaceModel
        objectName: "workspaceListModel"

        ListElement { workspaceKey: "studio"; workspaceName: qsTr("Design studio"); owner: "Maya Chen"; projectCount: 12; location: "/Workspaces/Design"; summary: qsTr("Shared design systems, prototypes, and product explorations.") }
        ListElement { workspaceKey: "research"; workspaceName: qsTr("Research library"); owner: "Alex Morgan"; projectCount: 8; location: "/Workspaces/Research"; summary: qsTr("Interview notes, usability studies, and reference material.") }
        ListElement { workspaceKey: "engineering"; workspaceName: qsTr("Engineering"); owner: "Sam Rivera"; projectCount: 21; location: "/Workspaces/Engineering"; summary: qsTr("Component specifications, implementation plans, and release work.") }
        ListElement { workspaceKey: "brand"; workspaceName: qsTr("Brand assets"); owner: "Noor Patel"; projectCount: 6; location: "/Workspaces/Brand"; summary: qsTr("Identity guidelines, campaign artwork, and approved assets.") }
        ListElement { workspaceKey: "mobile"; workspaceName: qsTr("Mobile experience"); owner: "Jordan Lee"; projectCount: 10; location: "/Workspaces/Mobile"; summary: qsTr("Mobile journeys, screen layouts, and interaction studies.") }
        ListElement { workspaceKey: "desktop"; workspaceName: qsTr("Desktop experience"); owner: "Taylor Kim"; projectCount: 14; location: "/Workspaces/Desktop"; summary: qsTr("Desktop workflows, window layouts, and keyboard interactions.") }
        ListElement { workspaceKey: "content"; workspaceName: qsTr("Content team"); owner: "Robin James"; projectCount: 7; location: "/Workspaces/Content"; summary: qsTr("Product language, help articles, and editorial planning.") }
        ListElement { workspaceKey: "analytics"; workspaceName: qsTr("Analytics"); owner: "Chris Park"; projectCount: 9; location: "/Workspaces/Analytics"; summary: qsTr("Dashboards, metric definitions, and reporting templates.") }
        ListElement { workspaceKey: "operations"; workspaceName: qsTr("Operations"); owner: "Casey Wilson"; projectCount: 5; location: "/Workspaces/Operations"; summary: qsTr("Team processes, planning documents, and service playbooks.") }
        ListElement { workspaceKey: "learning"; workspaceName: qsTr("Learning center"); owner: "Jamie Brooks"; projectCount: 11; location: "/Workspaces/Learning"; summary: qsTr("Workshop materials, tutorials, and onboarding resources.") }
        ListElement { workspaceKey: "community"; workspaceName: qsTr("Community"); owner: "Drew Ellis"; projectCount: 4; location: "/Workspaces/Community"; summary: qsTr("Community events, contributor resources, and feedback themes.") }
        ListElement { workspaceKey: "archive"; workspaceName: qsTr("Project archive"); owner: "Morgan Bell"; projectCount: 32; location: "/Workspaces/Archive"; summary: qsTr("Completed projects and reference snapshots from past releases.") }
    }

    GalleryDemo {
        title: qsTr("Workspaces")

        UTListView {
            id: workspaceList
            objectName: "workspaceListView"
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: 600
            Layout.preferredHeight: 184
            model: workspaceModel
            selectedIndex: 0

            onItemInvoked: function(index) {
                selectedIndex = index
            }

            delegate: Item {
                id: workspaceRow
                objectName: "workspaceListDelegate_" + workspaceKey
                required property int index
                required property string workspaceKey
                required property string workspaceName
                required property string owner
                required property int projectCount

                width: workspaceList.contentAvailableWidth
                height: 56

                UTToolButton {
                    id: workspaceButton
                    objectName: "workspaceListButton_" + workspaceRow.workspaceKey
                    anchors.fill: parent
                    anchors.margins: 4
                    focusPolicy: Qt.NoFocus
                    activeFocusOnTab: false
                    text: workspaceRow.workspaceName
                    leftPadding: 12
                    rightPadding: 12
                    topPadding: 4
                    bottomPadding: 4
                    backgroundColorEnum: UIColorToken.Listview_Item_Background
                    readonly property bool selected: workspaceList.selectedIndex === workspaceRow.index

                    contentItem: ColumnLayout {
                        spacing: 2

                        UTLabel {
                            Layout.fillWidth: true
                            text: workspaceRow.workspaceName
                            fontEnum: UIFontToken.Body_Text_Medium
                            colorEnum: UIColorToken.Listview_Item_Text
                            colorState: workspaceButton.selected ? UIColorState.Selected : UIColorState.Normal
                            elide: Text.ElideRight
                        }
                        UTLabel {
                            Layout.fillWidth: true
                            text: qsTr("%1 · %2 projects").arg(workspaceRow.owner).arg(workspaceRow.projectCount)
                            fontEnum: UIFontToken.Caption_Text
                            colorEnum: UIColorToken.Content_Secondary_Text
                            elide: Text.ElideRight
                        }
                    }

                    UTFocusItem {
                        objectName: "workspaceListFocus_" + workspaceRow.workspaceKey
                        delegateFocused: workspaceList.listView.activeFocus
                                         && workspaceList.currentIndex === workspaceRow.index
                        focusRadius: workspaceButton.radius
                    }

                    onClicked: {
                        workspaceList.focusOnItem(workspaceRow.index)
                        workspaceList.itemInvoked(workspaceRow.index)
                    }
                }
            }
        }

        ColumnLayout {
            objectName: "workspaceListSelectionDetails"
            Layout.fillWidth: true
            spacing: 4

            UTLabel {
                objectName: "workspaceListSelectedName"
                Layout.fillWidth: true
                text: root.selectedWorkspace ? root.selectedWorkspace.workspaceName : ""
                fontEnum: UIFontToken.Body_Text_Medium
                wrapMode: Text.Wrap
            }
            UTLabel {
                objectName: "workspaceListSelectedSummary"
                Layout.fillWidth: true
                text: root.selectedWorkspace ? root.selectedWorkspace.summary : ""
                wrapMode: Text.Wrap
            }
            UTLabel {
                objectName: "workspaceListSelectedLocation"
                Layout.fillWidth: true
                text: root.selectedWorkspace ? root.selectedWorkspace.location : ""
                colorEnum: UIColorToken.Content_Secondary_Text
                wrapMode: Text.WrapAnywhere
            }
        }
    }
}
