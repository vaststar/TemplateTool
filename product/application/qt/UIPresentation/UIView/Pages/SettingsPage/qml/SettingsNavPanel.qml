import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import UIView 1.0
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root
    property SettingsPageController controller

    width: 200

    Rectangle {
        anchors.fill: parent
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Main_Window_Background, UIColorState.Normal)
    }

    // Right border
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: UTComponentUtil.getPlainUIColor(UIColorToken.Sidebar_Border, UIColorState.Normal)
    }

    // Tree container
    UTTreeView {
        id: treeContainer
        anchors.fill: parent
        model: controller ? controller.treeModel : null
        expandAll: true

        onItemInvoked: function(idx) {
            var nodeId = treeView.model.data(idx, Qt.UserRole + 1);
            if (nodeId) controller.selectNode(nodeId);
        }

        delegate: UTTreeViewDelegate {
            text: model.title
            highlighted: controller && controller.currentNodeId === model.nodeId
            onClicked: {
                if (hasChildren) treeView.toggleExpanded(row);
                controller.selectNode(model.nodeId);
            }
        }
    }
}
