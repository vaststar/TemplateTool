#include "AppUpgrade/AppUpgradeController.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <commonhead/viewmodels/UpgradeViewModel/IUpgradeViewModel.h>

#include "UIWindowUtilities/WindowGeometry.h"
#include <UIViewModelSignalBridge/emitters/UpgradeViewModelEmitter.h>
#include "UIEvents/UIUpgradeEvent.h"
#include "LoggerDefine.h"

namespace {
// QML resource paths used by this controller.
const QString kUpgradeDialogQml = QStringLiteral("UIView/AppUpgrade/qml/UpgradeDialog.qml");
}

AppUpgradeController::AppUpgradeController(QObject* parent)
    : UIViewController(parent)
    , m_emitter(std::make_shared<UIViewModelSignalBridge::UpgradeViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("AppUpgradeController constructed, address: " << this);
}

AppUpgradeController::~AppUpgradeController()
{
    UIVIEW_LOG_DEBUG("AppUpgradeController destroying, address: " << this);
}

void AppUpgradeController::init()
{
    // Listen for upgrade events from EventBus
    listenUIEvents<UIUpgradeEvent>();

    // Connect emitter signals
    using Emitter = UIViewModelSignalBridge::UpgradeViewModelEmitter;
    connect(m_emitter.get(), &Emitter::signals_onCheckCompleted,
            this, &AppUpgradeController::onCheckCompleted);
    connect(m_emitter.get(), &Emitter::signals_onDownloadProgress,
            this, &AppUpgradeController::onDownloadProgress);
    connect(m_emitter.get(), &Emitter::signals_onUpgradeStateChanged,
            this, &AppUpgradeController::onUpgradeStateChanged);
    connect(m_emitter.get(), &Emitter::signals_onUpgradeError,
            this, &AppUpgradeController::onUpgradeError);
    // Opening belongs to the GUI thread. Queue even for GUI-originated actions
    // so the service can finish its OpeningInstaller transition first.
    connect(m_emitter.get(), &Emitter::signals_onInstallerOpenRequested,
            this, &AppUpgradeController::onInstallerOpenRequested, Qt::QueuedConnection);
    // Quit after the service has handed off the installer and released cleanup ownership.
    connect(m_emitter.get(), &Emitter::signals_onUpgradeStateChanged,
            this, [this](commonHead::viewModels::model::UpgradeViewState state) {
                if (state == commonHead::viewModels::model::UpgradeViewState::InstallerOpened)
                    emit quitRequested();
            }, Qt::QueuedConnection);

    // Create ViewModel
    m_viewModel = getViewModelFactory()->createUpgradeViewModelInstance();
    m_viewModel->registerCallback(m_emitter);
    m_viewModel->initViewModel();
}

// ============================================================================
// Property Getters — all derived from m_state
// ============================================================================

bool AppUpgradeController::isChecking() const
{
    return m_state == commonHead::viewModels::model::UpgradeViewState::Checking;
}

bool AppUpgradeController::hasUpgrade() const
{
    return m_state == commonHead::viewModels::model::UpgradeViewState::UpgradeAvailable;
}

bool AppUpgradeController::isDownloading() const
{
    return m_state == commonHead::viewModels::model::UpgradeViewState::Downloading;
}

bool AppUpgradeController::isVerifying() const
{
    return m_state == commonHead::viewModels::model::UpgradeViewState::Verifying;
}

bool AppUpgradeController::isReadyToInstall() const
{
    return m_state == commonHead::viewModels::model::UpgradeViewState::ReadyToInstall;
}

bool AppUpgradeController::isOpeningInstaller() const
{
    return m_state == commonHead::viewModels::model::UpgradeViewState::OpeningInstaller;
}

bool AppUpgradeController::isInstallerOpened() const
{
    return m_state == commonHead::viewModels::model::UpgradeViewState::InstallerOpened;
}

bool AppUpgradeController::isIdle() const
{
    return m_state == commonHead::viewModels::model::UpgradeViewState::Idle;
}

QString AppUpgradeController::version() const
{
    return m_version;
}

QString AppUpgradeController::releaseNotes() const
{
    return m_releaseNotes;
}

bool AppUpgradeController::isMandatory() const
{
    return m_mandatory;
}

double AppUpgradeController::downloadProgress() const
{
    return m_downloadProgress;
}

QString AppUpgradeController::errorMessage() const
{
    return m_errorMessage;
}

// ============================================================================
// Q_INVOKABLE Actions
// ============================================================================

void AppUpgradeController::downloadUpgrade()
{
    if (m_viewModel)
    {
        m_viewModel->downloadUpgrade();
    }
}

void AppUpgradeController::openInstaller()
{
    if (m_viewModel)
    {
        m_errorMessage.clear();
        emit errorChanged();
        m_viewModel->openInstaller();
    }
}

void AppUpgradeController::revealInstaller()
{
    if (m_installerPath.isEmpty())
        return;

    const auto directory = QFileInfo(m_installerPath).absolutePath();
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(directory)))
    {
        onUpgradeError(tr("Could not open the installer folder. Installer path: %1")
                           .arg(m_installerPath));
    }
}

void AppUpgradeController::cancelDownload()
{
    if (m_viewModel)
    {
        m_viewModel->cancelDownload();
    }
}

void AppUpgradeController::dismiss()
{
    using S = commonHead::viewModels::model::UpgradeViewState;

    UIVIEW_LOG_DEBUG("dismiss (state=" << static_cast<int>(m_state) << ")");

    m_dialogOpen = false;
    m_errorMessage.clear();
    emit errorChanged();

    // Clear the cached result when closing a settled upgrade dialog. The service
    // keeps an installer already handed to the operating system, so returning
    // from InstallerOpened to Idle must not remove that file.
    if (m_viewModel && (m_state == S::UpgradeAvailable
                        || m_state == S::ReadyToInstall
                        || m_state == S::InstallerOpened
                        || m_state == S::Failed))
    {
        m_viewModel->dismissUpgrade();
    }
}

// ============================================================================
// Event Handling — UIUpgradeEvent from EventBus
// ============================================================================

bool AppUpgradeController::event(QEvent* e)
{
    if (e->type() == UIUpgradeEvent::eventType())
    {
        auto* upgradeEvent = static_cast<UIUpgradeEvent*>(e);
        switch (upgradeEvent->mAction)
        {
        case UIUpgradeEvent::Action::CheckForUpgrade:
            UIVIEW_LOG_DEBUG("UIUpgradeEvent::CheckForUpgrade");
            if (isOperationInProgress())
            {
                // Keep the current download / verification / installer handoff visible
                showUpgradeDialog();
            }
            else if (m_viewModel)
            {
                // Any other state (Idle, UpgradeAvailable, Failed) → start a fresh check.
                // The FSM handles re-check from all these states.
                m_errorMessage.clear();
                emit errorChanged();
                showUpgradeDialog();
                m_viewModel->checkForUpgrade();
            }
            break;
        }
        return true;
    }
    return UIViewController::event(e);
}

bool AppUpgradeController::isOperationInProgress() const
{
    using S = commonHead::viewModels::model::UpgradeViewState;
    return m_state == S::Checking
        || m_state == S::Downloading
        || m_state == S::Verifying
        || m_state == S::ReadyToInstall
        || m_state == S::OpeningInstaller
        || m_state == S::InstallerOpened;
}

// ============================================================================
// ViewModel Callback Slots
// ============================================================================

void AppUpgradeController::onCheckCompleted(bool hasUpgrade, const QString& version,
                                            const QString& releaseNotes, bool mandatory)
{
    UIVIEW_LOG_DEBUG("onCheckCompleted: hasUpgrade=" << hasUpgrade
                     << ", version=" << version.toStdString());
    m_version = version;
    m_releaseNotes = releaseNotes;
    m_mandatory = mandatory;
    emit upgradeInfoChanged();
    // Note: dialog visibility is owned by event() (user-triggered check) or by
    // onUpgradeStateChanged (auto-triggered check, when state becomes
    // UpgradeAvailable). Don't reopen the dialog here.
}

void AppUpgradeController::onDownloadProgress(int64_t currentBytes, int64_t totalBytes)
{
    m_downloadProgress = (totalBytes > 0)
        ? static_cast<double>(currentBytes) / static_cast<double>(totalBytes)
        : 0.0;
    emit progressChanged();
}

void AppUpgradeController::onUpgradeStateChanged(commonHead::viewModels::model::UpgradeViewState state)
{
    using State = commonHead::viewModels::model::UpgradeViewState;

    UIVIEW_LOG_DEBUG("onUpgradeStateChanged: " << static_cast<int>(state));

    m_state = state;

    if (state == State::Downloading)
        m_downloadProgress = 0.0;

    emit stateChanged();
}

void AppUpgradeController::onUpgradeError(const QString& message)
{
    UIVIEW_LOG_DEBUG("onUpgradeError: " << message.toStdString());
    m_errorMessage = message;
    emit stateChanged();
    emit errorChanged();
}

void AppUpgradeController::onInstallerOpenRequested(const QString& installerPath)
{
    if (!m_viewModel)
        return;

    m_installerPath = installerPath;
    const bool opened = QDesktopServices::openUrl(QUrl::fromLocalFile(installerPath));
    // True means the operating system accepted the request, not that installation
    // finished. Report it before requesting application shutdown.
    m_viewModel->reportInstallerOpenResult(opened);
}

// ============================================================================
// Dialog
// ============================================================================

void AppUpgradeController::showUpgradeDialog()
{
    if (m_dialogOpen)
    {
        UIVIEW_LOG_DEBUG("showUpgradeDialog skipped — already open");
        return;
    }
    UIVIEW_LOG_DEBUG("showUpgradeDialog");
    m_dialogOpen = true;
    auto win = getViewFactory()->createQmlWindow(
        kUpgradeDialogQml,
        { { QStringLiteral("controller"), QVariant::fromValue<QObject*>(this) } });
    if (!win)
    {
        m_dialogOpen = false;
        return;
    }
    UIUtilities::WindowGeometry::centerOnParentWhenShown(win);
    win->show();
}
