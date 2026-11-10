import QtQuick
import Qt.labs.qmlmodels
import UTComponent 1.0
import UIResourceLoader 1.0

Item {
    id: root

    property int currentPage: 0
    signal pageSelected(int page)

    TreeModel {
        id: navigationModel
        TableModelColumn { display: "title" }

        rows: [
            {
                title: "UTComponent", page: -1,
                rows: [
                    { title: "Buttons", page: 0 },
                    { title: "Toggles", page: 1 },
                    { title: "Inputs", page: 2 },
                    { title: "Pickers", page: 3 },
                    { title: "Display", page: 4 },
                    { title: "Containers", page: 5 }
                ]
            },
            {
                title: "UTComposite", page: -1,
                rows: [
                    { title: "Pickers", page: 6 }
                ]
            }
        ]
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
