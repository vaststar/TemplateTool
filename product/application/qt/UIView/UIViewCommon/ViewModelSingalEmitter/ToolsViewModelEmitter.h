#pragma once

#include <QObject>
#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>

namespace UIVMSignalEmitter {

class ToolsViewModelEmitter : public QObject,
                              public commonHead::viewModels::IToolsViewModelCallback
{
    Q_OBJECT
public:
    explicit ToolsViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    void onToolsTreeChanged(const commonHead::viewModels::model::ToolsTreePtr& tree) override
    {
        emit signals_onToolsTreeChanged(tree);
    }

    void onToolsTreeStructureChanged(const commonHead::viewModels::model::ToolsTreeNodeChange& change) override
    {
        emit signals_onToolsTreeStructureChanged(change);
    }

    void onToolsTreeItemsUpdated() override
    {
        emit signals_onToolsTreeItemsUpdated();
    }

    void onToolsTreeItemUpdated(const std::string& nodeId) override
    {
        emit signals_onToolsTreeItemUpdated(QString::fromStdString(nodeId));
    }

    void onCurrentToolNodeChanged(const std::string& nodeId,
                                  commonHead::viewModels::model::ToolPanelType panelType) override
    {
        emit signals_onCurrentToolNodeChanged(
            QString::fromStdString(nodeId), static_cast<int>(panelType));
    }

signals:
    void signals_onToolsTreeChanged(const commonHead::viewModels::model::ToolsTreePtr& tree);
    void signals_onToolsTreeStructureChanged(const commonHead::viewModels::model::ToolsTreeNodeChange& change);
    void signals_onToolsTreeItemsUpdated();
    void signals_onToolsTreeItemUpdated(const QString& nodeId);
    void signals_onCurrentToolNodeChanged(const QString& nodeId, int panelType);
};

} // namespace UIVMSignalEmitter
