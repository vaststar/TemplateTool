#pragma once

#include <QAbstractItemModel>
#include <QtQml>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

// Pure tree model. Owns its own mirror of nodes; exposes coarse mutation
// methods so callers (controller) can translate VM callbacks into proper
// QAbstractItemModel structural changes.
class ContactListItemModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    using NodeData     = commonHead::viewModels::model::ContactNodeData;
    using RelationData = commonHead::viewModels::model::ContactRelationData;
    using TreePtr      = std::shared_ptr<commonHead::viewModels::model::IContactTree>;
    using TreeNodePtr  = std::shared_ptr<commonHead::viewModels::model::IContactTreeNode>;

    explicit ContactListItemModel(QObject* parent = nullptr);
    ~ContactListItemModel() override;

    // --- Mutation API (each method emits the proper begin/end pair(s)) ---
    void resetFromTree(const TreePtr& tree);                                       // beginResetModel/endResetModel
    void insertNodes(const std::vector<NodeData>& datas);                          // beginInsertRows under root
    void updateNodes(const std::vector<NodeData>& datas);                          // dataChanged
    void removeNodes(const std::vector<std::string>& ids);                         // children → root, then remove
    void setParents(const std::vector<std::pair<std::string, std::string>>& pairs);// beginMoveRows
    void clearParents(const std::vector<std::string>& childIds);                   // beginMoveRows → root

    // --- QAbstractItemModel ---
    QVariant      data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant      headerData(int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole) const override;
    QModelIndex   index(int row, int column,
                        const QModelIndex& parent = {}) const override;
    QModelIndex   parent(const QModelIndex& index) const override;
    int           rowCount(const QModelIndex& parent = {}) const override;
    int           columnCount(const QModelIndex& parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Node {
        NodeData    data;
        std::string parentId;              // "" = root sentinel
        int         rowInParent = -1;      // -1 for root
        std::vector<std::string> childIds; // ordered
    };

    Node* getRoot() const;
    Node* findNode(const std::string& id) const;
    Node* nodeFromIndex(const QModelIndex& idx) const;
    QModelIndex indexFor(Node* node) const;

    void resetMirror();
    void walkPopulate(const TreeNodePtr& src, const std::string& parentId);
    void moveNodeToParent(const std::string& childId, const std::string& newParentId);

private:
    std::unordered_map<std::string, std::unique_ptr<Node>> m_nodes; // includes "" → root
};
