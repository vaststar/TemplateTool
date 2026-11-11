import QtQuick
import Qt.labs.qmlmodels
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root

    property var rows: []
    property int currentPage: 0
    signal pageSelected(int page)

    TreeModel {
        id: navigationModel
        TableModelColumn { display: "title" }

        rows: root.rows
    }

    UTTreeView {
        id: navigationTree
        anchors.fill: parent
        anchors.topMargin: 12
        model: navigationModel
        expandAll: true

        function activate(index) {
            if (!index.valid)
                return
            const node = navigationModel.getRow(index)
            if (node.page < 0)
                treeView.toggleExpanded(treeView.rowAtIndex(index))
            else
                root.pageSelected(node.page)
        }

        onItemInvoked: function(index) { activate(index) }

        delegate: UTTreeViewDelegate {
            readonly property int page: {
                if (!treeView)
                    return -1
                const index = treeView.index(row, 0)
                return index.valid ? navigationModel.getRow(index).page : -1
            }

            backgroundColorEnum: UIColorToken.Content_Nav_Item_Background
            backgroundRadius: 6
            textColorEnum: UIColorToken.Listview_Item_Text
            text: model.display
            highlighted: page >= 0 && root.currentPage === page
            onClicked: navigationTree.activate(treeView.index(row, 0))
        }
    }
}
