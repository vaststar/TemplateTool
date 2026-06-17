#include "AppUpgrade/AppUpgradeController.h"

#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <commonHead/viewModels/UpgradeViewModel/IUpgradeViewModel.h>

#include "UIViewHelper/UIViewHelper.h"
#include "ViewModelSingalEmitter/UpgradeViewModelEmitter.h"
#include "UIEvents/UIUpgradeEvent.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace {
// QML resource paths used by this controller.
const QString kUpgradeDialogQml = QStringLiteral("UIView/AppUpgrade/qml/UpgradeDialog.qml");
}

AppUpgradeController::AppUpgradeController(QObject* parent)
    : UIViewController(parent)
    , m_emitter(std::make_shared<UIVMSignalEmitter::UpgradeViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create AppUpgradeController");
}

AppUpgradeController::~AppUpgradeController()
{
    UIVIEW_LOG_DEBUG("destroy AppUpgradeController");
}

void AppUpgradeController::init()
{
    UIVIEW_LOG_DEBUG("AppUpgradeController::init");

    // Listen for upgrade events from EventBus
    listenUIEvents<UIUpgradeEvent>();

    // Connect emitter signals
    using Emitter = UIVMSignalEmitter::UpgradeViewModelEmitter;
    connect(m_emitter.get(), &Emitter::signals_onCheckCompleted,
            this, &AppUpgradeController::onCheckCompleted);
    connect(m_emitter.get(), &Emitter::signals_onDownloadProgress,
            this, &AppUpgradeController::onDownloadProgress);
    connect(m_emitter.get(), &Emitter::signals_onUpgradeStateChanged,
            this, &AppUpgradeController::onUpgradeStateChanged);
    connect(m_emitter.get(), &Emitter::signals_onUpgradeError,
            this, &AppUpgradeController::onUpgradeError);

    // Create ViewModel
    m_viewModel = getAppContext()->getViewModelFactory()->createUpgradeViewModelInstance();
    m_viewModel->initViewModel();
    m_viewModel->registerCallback(m_emitter);

    UIVIEW_LOG_DEBUG("AppUpgradeController::init done");
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
    using S = commonHead::viewModels::model::UpgradeViewState;
    return m_state == S::Verifying || m_state == S::Extracting;
}

bool AppUpgradeController::isReadyToInstall() const
{
    return m_state == commonHead::viewModels::model::UpgradeViewState::ReadyToInstall;
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

void AppUpgradeController::installAndRestart()
{
    if (m_viewModel)
    {
        m_viewModel->installAndRestart();
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

    // If the user is dismissing while we're parked on a state where the FSM
    // is holding onto upgrade artifacts (available info / extracted staging
    // dir / failure data), tell it to drop them and return to Idle. Otherwise
    // the next CheckForUpgrade would either be ignored or re-show the cached
    // result instead of querying the server.
    if (m_viewModel && (m_state == S::UpgradeAvailable
                        || m_state == S::ReadyToInstall
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
    if (e->type() == UIUpgradeEvent::type)
    {
        auto* upgradeEvent = static_cast<UIUpgradeEvent*>(e);
        switch (upgradeEvent->mAction)
        {
        case UIUpgradeEvent::Action::CheckForUpgrade:
            UIVIEW_LOG_DEBUG("UIUpgradeEvent::CheckForUpgrade");
            if (isOperationInProgress())
            {
                // Download / verify / install in flight — just bring the dialog back
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
        || m_state == S::Extracting
        || m_state == S::ReadyToInstall;
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

    if (state == State::Installing)
    {
        UIVIEW_LOG_DEBUG("Updater launched, requesting app close");
        emit quitRequested();
    }
}

void AppUpgradeController::onUpgradeError(const QString& message)
{
    UIVIEW_LOG_DEBUG("onUpgradeError: " << message.toStdString());
    m_errorMessage = message;
    emit stateChanged();
    emit errorChanged();
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
    auto win = getAppContext()->getViewFactory()->createQmlWindow(
        kUpgradeDialogQml,
        { { QStringLiteral("controller"), QVariant::fromValue<QObject*>(this) } });
    if (!win)
    {
        m_dialogOpen = false;
        return;
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}
