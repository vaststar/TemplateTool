#pragma once

#include <QObject>
#include <commonHead/viewModels/SettingsViewModel/ISettingsViewModel.h>

namespace UIVMSignalEmitter {

class SettingsViewModelEmitter : public QObject,
                                 public commonHead::viewModels::ISettingsViewModelCallback
{
    Q_OBJECT
public:
    explicit SettingsViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    void onSettingsTreeChanged(const commonHead::viewModels::model::SettingsTreePtr& tree) override
    {
        emit signals_onSettingsTreeChanged(tree);
    }

    void onCurrentSettingsNodeChanged(const std::string& nodeId,
                                      commonHead::viewModels::model::SettingsPanelType panelType) override
    {
        emit signals_onCurrentSettingsNodeChanged(
            QString::fromStdString(nodeId), static_cast<int>(panelType));
    }

signals:
    void signals_onSettingsTreeChanged(const commonHead::viewModels::model::SettingsTreePtr& tree);
    void signals_onCurrentSettingsNodeChanged(const QString& nodeId, int panelType);
};

} // namespace UIVMSignalEmitter
