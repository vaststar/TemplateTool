#pragma once

#include <QAbstractItemModel>
#include <QtQml>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>

// Pure tree model. Owns its own mirror of nodes; exposes coarse mutation
// methods so callers (controller) can translate VM callbacks into proper
// QAbstractItemModel structural changes. Mirrors ContactListItemModel but
// drops drag/drop hooks (no setParents/clearParents) because Settings is a
// strict, read-only tree on the UI side.
class SettingsTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        NodeIdRole = Qt::UserRole + 1,
        TitleRole,
        IconRole,
        PanelTypeRole,
    };

    using NodeData    = commonHead::viewModels::model::SettingsNodeData;
    using TreePtr     = std::shared_ptr<commonHead::viewModels::model::ISettingsTree>;
    using TreeNodePtr = std::shared_ptr<commonHead::viewModels::model::ISettingsTreeNode>;

    explicit SettingsTreeModel(QObject* parent = nullptr);
    ~SettingsTreeModel() override;

    // --- Mutation API (each method emits the proper begin/end pair(s)) ---
    void resetFromTree(const TreePtr& tree);                   // beginResetModel/endResetModel
    void insertNodes(const std::vector<NodeData>& datas);      // beginInsertRows per parent
    void updateNodes(const std::vector<NodeData>& datas);      // dataChanged
    void removeNodes(const std::vector<std::string>& ids);     // beginRemoveRows

    Q_INVOKABLE QModelIndex indexOfId(const QString& id) const;

    // --- QAbstractItemModel ---
    QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
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

private:
    std::unordered_map<std::string, std::unique_ptr<Node>> m_nodes; // includes "" → root
};
