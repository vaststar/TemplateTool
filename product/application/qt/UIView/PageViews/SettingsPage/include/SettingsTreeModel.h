#pragma once

#include <QAbstractItemModel>
#include <QtQml>

namespace commonHead::viewModels {
    class ISettingsViewModel;

    namespace model {
        class ISettingsTreeNode;
    }
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

    void setViewModel(const std::shared_ptr<commonHead::viewModels::ISettingsViewModel>& viewModel);

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

    std::shared_ptr<commonHead::viewModels::ISettingsViewModel> m_viewModel;
};
