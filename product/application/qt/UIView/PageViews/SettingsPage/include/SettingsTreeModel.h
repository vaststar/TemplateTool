#pragma once

#include <QAbstractItemModel>
#include <QtQml>
#include <string>

namespace commonHead::viewModels::model {
    class ISettingsTree;
    class ISettingsTreeNode;
    struct SettingsTreeNodeChange;
}

class SettingsTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        NodeIdRole = Qt::UserRole + 1,
        TitleRole,
        IconRole,
        PanelTypeRole
    };

    explicit SettingsTreeModel(QObject* parent = nullptr);
    ~SettingsTreeModel() override;

    /// L4: Replace entire tree
    void setTree(const std::shared_ptr<commonHead::viewModels::model::ISettingsTree>& tree);

    /// L3: Apply a single structural change (insert/remove)
    void applyStructureChange(const commonHead::viewModels::model::SettingsTreeNodeChange& change);

    /// L2: Notify that all item properties changed in-place
    void notifyAllItemsChanged();

    /// L1: Notify that a single item's properties changed in-place
    void notifyItemChanged(const std::string& nodeId);

    // QAbstractItemModel interface
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    commonHead::viewModels::model::ISettingsTreeNode* nodeFromIndex(const QModelIndex& index) const;
    QModelIndex indexForNodeId(const std::string& nodeId) const;

    std::shared_ptr<commonHead::viewModels::model::ISettingsTree> m_tree;
};
