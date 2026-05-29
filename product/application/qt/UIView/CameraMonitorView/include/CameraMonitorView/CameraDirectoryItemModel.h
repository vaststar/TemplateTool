#pragma once

#include <memory>

#include <QAbstractItemModel>
#include <QtQml>

namespace commonHead::viewModels::model {
    class ICameraDirectoryTree;
    class ICameraDirectoryTreeNode;
}

// QAbstractItemModel adapter over commonHead::viewModels::model::ICameraDirectoryTree。
// 设计与 ContactsPage::ContactListItemModel 一致：树快照 + reset 语义。
// 增量事件由上层 controller 重新 setTree() 触发 begin/endResetModel。
class CameraDirectoryItemModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        DisplayNameRole,
        NodeTypeRole,   // 0 = Group, 1 = Camera
        StatusRole      // CameraNodeStatus int
    };

    explicit CameraDirectoryItemModel(QObject* parent = nullptr);
    ~CameraDirectoryItemModel() override;

    void setTree(const std::shared_ptr<commonHead::viewModels::model::ICameraDirectoryTree>& tree);

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    commonHead::viewModels::model::ICameraDirectoryTreeNode* nodeFromIndex(const QModelIndex& index) const;

private:
    std::shared_ptr<commonHead::viewModels::model::ICameraDirectoryTree> mTree;
};
