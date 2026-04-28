#pragma once

#include <QObject>
#include <QPointer>
#include <memory>

#include "UIViewCommon/UIViewBase/include/UIViewController.h"

#include <commonHead/viewModels/UpgradeViewModel/IUpgradeViewModel.h>

namespace commonHead::viewModels {
    class IUpgradeViewModel;
}

namespace UIVMSignalEmitter {
    class UpgradeViewModelEmitter;
}

class AppUpgradeController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(bool checking READ isChecking NOTIFY stateChanged)
    Q_PROPERTY(bool hasUpgrade READ hasUpgrade NOTIFY stateChanged)
    Q_PROPERTY(bool downloading READ isDownloading NOTIFY stateChanged)
    Q_PROPERTY(bool verifying READ isVerifying NOTIFY stateChanged)
    Q_PROPERTY(bool readyToInstall READ isReadyToInstall NOTIFY stateChanged)
    Q_PROPERTY(QString version READ version NOTIFY upgradeInfoChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY upgradeInfoChanged)
    Q_PROPERTY(bool mandatory READ isMandatory NOTIFY upgradeInfoChanged)
    Q_PROPERTY(double downloadProgress READ downloadProgress NOTIFY progressChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)

public:
    explicit AppUpgradeController(QObject* parent = nullptr);
    ~AppUpgradeController() override;

    bool isChecking() const;
    bool hasUpgrade() const;
    bool isDownloading() const;
    bool isVerifying() const;
    bool isReadyToInstall() const;
    QString version() const;
    QString releaseNotes() const;
    bool isMandatory() const;
    double downloadProgress() const;
    QString errorMessage() const;

    Q_INVOKABLE void downloadUpgrade();
    Q_INVOKABLE void installAndRestart();
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void dismiss();

signals:
    void stateChanged();
    void upgradeInfoChanged();
    void progressChanged();
    void errorChanged();
    void quitRequested();

protected:
    void init() override;
    bool event(QEvent* e) override;

private slots:
    void onCheckCompleted(bool hasUpgrade, const QString& version, const QString& releaseNotes, bool mandatory);
    void onDownloadProgress(int64_t currentBytes, int64_t totalBytes);
    void onUpgradeStateChanged(commonHead::viewModels::model::UpgradeViewState state);
    void onUpgradeError(const QString& message);

private:
    void showUpgradeDialog();

    std::shared_ptr<commonHead::viewModels::IUpgradeViewModel> m_viewModel;
    std::shared_ptr<UIVMSignalEmitter::UpgradeViewModelEmitter> m_emitter;

    bool m_checking = false;
    bool m_hasUpgrade = false;
    bool m_downloading = false;
    bool m_verifying = false;
    bool m_readyToInstall = false;
    bool m_dialogOpen = false;
    QString m_version;
    QString m_releaseNotes;
    bool m_mandatory = false;
    double m_downloadProgress = 0.0;
    QString m_errorMessage;
};
