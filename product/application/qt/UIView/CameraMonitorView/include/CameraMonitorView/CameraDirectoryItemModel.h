#pragma once

#include <QAbstractItemModel>
#include <QtQml>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryTreeModel.h>

// Tree model with an internal node mirror. Mutation methods translate VM
// callbacks into proper QAbstractItemModel structural changes.
class CameraDirectoryItemModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    using NodeData     = commonHead::viewModels::model::CameraDirectoryNodeData;
    using RelationData = commonHead::viewModels::model::CameraDirectoryRelationData;
    using TreePtr      = std::shared_ptr<commonHead::viewModels::model::ICameraDirectoryTree>;
    using TreeNodePtr  = std::shared_ptr<commonHead::viewModels::model::ICameraDirectoryTreeNode>;

    enum Roles {
        IdRole = Qt::UserRole + 1,
        DisplayNameRole,
        NodeTypeRole,   // 0 = Group, 1 = Camera
        StatusRole      // CameraNodeStatus int
    };

    explicit CameraDirectoryItemModel(QObject* parent = nullptr);
    ~CameraDirectoryItemModel() override;

    // Each mutator emits the proper begin/end signal pairs.
    void resetFromTree(const TreePtr& tree);
    void insertNodes(const std::vector<NodeData>& datas);
    void updateNodes(const std::vector<NodeData>& datas);
    void removeNodes(const std::vector<std::string>& ids);
    void setParents(const std::vector<std::pair<std::string, std::string>>& pairs);
    void clearParents(const std::vector<std::string>& childIds);

    Q_INVOKABLE QModelIndex indexOfId(const QString& id) const;

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
        std::vector<std::string> childIds;
    };

    Node* getRoot() const;
    Node* findNode(const std::string& id) const;
    Node* nodeFromIndex(const QModelIndex& idx) const;
    QModelIndex indexFor(Node* node) const;

    void resetMirror();
    void walkPopulate(const TreeNodePtr& src, const std::string& parentId);
    void moveNodeToParent(const std::string& childId, const std::string& newParentId);

private:
    std::unordered_map<std::string, std::unique_ptr<Node>> m_nodes;
};
