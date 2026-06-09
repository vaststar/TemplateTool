#pragma once

#include "RegisterViewModelMetaTypes.h"

#include <QObject>
#include <string>
#include <vector>

#include <commonHead/viewModels/SettingsViewModel/ISettingsViewModel.h>

namespace UIVMSignalEmitter {

// Qt-signal bridge for ISettingsViewModelCallback.
class SettingsViewModelEmitter : public QObject,
                                 public commonHead::viewModels::ISettingsViewModelCallback
{
    Q_OBJECT
public:
    using NodeData = commonHead::viewModels::model::SettingsNodeData;

    explicit SettingsViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    void onSettingsTreeReady() override
    { emit signals_onSettingsTreeReady(); }

    void onSettingsNodesAdded(const std::vector<NodeData>& nodes) override
    { emit signals_onSettingsNodesAdded(nodes); }

    void onSettingsNodesUpdated(const std::vector<NodeData>& nodes) override
    { emit signals_onSettingsNodesUpdated(nodes); }

    void onSettingsNodesRemoved(const std::vector<std::string>& nodeIds) override
    { emit signals_onSettingsNodesRemoved(nodeIds); }

signals:
    void signals_onSettingsTreeReady();
    void signals_onSettingsNodesAdded  (const std::vector<NodeData>& nodes);
    void signals_onSettingsNodesUpdated(const std::vector<NodeData>& nodes);
    void signals_onSettingsNodesRemoved(const std::vector<std::string>& nodeIds);
};

} // namespace UIVMSignalEmitter
