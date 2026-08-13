#pragma once

#include <QAbstractItemModel>
#include <QtQml>

namespace commonHead::viewModels::model {
    class IJsonTree;
    class IJsonTreeNode;
}

class JsonTreeItemModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        KeyRole = Qt::UserRole + 1,
        ValueRole,
        NodeTypeRole,
        ChildCountRole,
        ShowCommaRole
    };

    explicit JsonTreeItemModel(QObject* parent = nullptr);
    ~JsonTreeItemModel() override;

    void setTree(const std::shared_ptr<commonHead::viewModels::model::IJsonTree>& tree);
    void clearTree();

    // QAbstractItemModel interface
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    commonHead::viewModels::model::IJsonTreeNode* nodeFromIndex(const QModelIndex& index) const;

    std::shared_ptr<commonHead::viewModels::model::IJsonTree> m_tree;
};
