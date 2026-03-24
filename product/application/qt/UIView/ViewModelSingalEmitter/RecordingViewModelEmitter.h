#pragma once

#include <QObject>
#include <QString>
#include <commonHead/viewModels/RecordingViewModel/IRecordingViewModel.h>

namespace UIVMSignalEmitter {

class RecordingViewModelEmitter : public QObject,
                                  public commonHead::viewModels::IRecordingViewModelCallback
{
    Q_OBJECT
public:
    explicit RecordingViewModelEmitter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    // ── IRecordingViewModelCallback overrides ──

    void onSettingsChanged(const commonHead::viewModels::model::RecordingSettings& settings) override
    {
        emit signals_onSettingsChanged(settings);
    }

    void onError(const std::string& message) override
    {
        emit signals_onError(QString::fromStdString(message));
    }

signals:
    void signals_onSettingsChanged(const commonHead::viewModels::model::RecordingSettings& settings);
    void signals_onError(const QString& message);
};

} // namespace UIVMSignalEmitter
