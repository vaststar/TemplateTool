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

    void onToolsTreeReady() override
    {
        emit signals_onToolsTreeReady();
    }

    void onToolsNodesAdded(const std::vector<commonHead::viewModels::model::ToolNodeData>& nodes) override
    {
        emit signals_onToolsNodesAdded(nodes);
    }

    void onToolsNodesUpdated(const std::vector<commonHead::viewModels::model::ToolNodeData>& nodes) override
    {
        emit signals_onToolsNodesUpdated(nodes);
    }

    void onToolsNodesRemoved(const std::vector<std::string>& nodeIds) override
    {
        emit signals_onToolsNodesRemoved(nodeIds);
    }

signals:
    void signals_onToolsTreeReady();
    void signals_onToolsNodesAdded(const std::vector<commonHead::viewModels::model::ToolNodeData>& nodes);
    void signals_onToolsNodesUpdated(const std::vector<commonHead::viewModels::model::ToolNodeData>& nodes);
    void signals_onToolsNodesRemoved(const std::vector<std::string>& nodeIds);
};

} // namespace UIVMSignalEmitter
