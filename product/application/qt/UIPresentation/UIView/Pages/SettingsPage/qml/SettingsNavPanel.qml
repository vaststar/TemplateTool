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

    // Tree container
    UTTreeView {
        id: treeContainer
        anchors.fill: parent
        anchors.topMargin: 12
        model: controller ? controller.treeModel : null
        expandAll: true

        onItemInvoked: function(idx) {
            var nodeId = treeView.model.data(idx, Qt.UserRole + 1);
            if (nodeId) controller.selectNode(nodeId);
        }

        delegate: UTTreeViewDelegate {
            backgroundColorEnum: UIColorToken.Content_Nav_Item_Background
            backgroundRadius: 6
            textColorEnum: UIColorToken.Listview_Item_Text
            text: model.title
            highlighted: controller && controller.currentNodeId === model.nodeId
            onClicked: {
                if (hasChildren) treeView.toggleExpanded(row);
                controller.selectNode(model.nodeId);
            }
        }
    }
}
