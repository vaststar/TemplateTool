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

    void onCurrentToolNodeChanged(const std::string& nodeId,
                                  commonHead::viewModels::model::ToolPanelType panelType) override
    {
        emit signals_onCurrentToolNodeChanged(
            QString::fromStdString(nodeId), static_cast<int>(panelType));
    }

signals:
    void signals_onToolsTreeChanged(const commonHead::viewModels::model::ToolsTreePtr& tree);
    void signals_onCurrentToolNodeChanged(const QString& nodeId, int panelType);
};

} // namespace UIVMSignalEmitter
