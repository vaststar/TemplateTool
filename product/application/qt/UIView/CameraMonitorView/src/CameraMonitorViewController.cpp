#include "CameraMonitorView/CameraMonitorViewController.h"

#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryViewModel.h>
#include <commonHead/viewModels/MediaCameraViewModel/CameraSource.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <UIFabrication/IUIViewFactory.h>

#include "MediaCameraView/MediaCameraViewController.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
#include "UIViewHelper/UIViewHelper.h"
#include "ViewModelSingalEmitter/CameraDirectoryViewModelEmitter.h"

CameraMonitorViewController::CameraMonitorViewController(QObject* parent)
    : UIViewController(parent)
    , mCameraDirectoryEmitter(std::make_shared<UIVMSignalEmitter::CameraDirectoryViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create CameraMonitorViewController");
}

CameraMonitorViewController::~CameraMonitorViewController()
{
    UIVIEW_LOG_DEBUG("delete CameraMonitorViewController");
}

QAbstractItemModel* CameraMonitorViewController::getCameraTreeModel() const  { return mCameraTreeModel; }
QString CameraMonitorViewController::getCurrentCameraId() const              { return mCurrentCameraId; }
QString CameraMonitorViewController::getCurrentCameraName() const            { return mCurrentCameraName; }
CameraMonitorViewController::LoadState CameraMonitorViewController::getLoadState() const { return mLoadState; }

void CameraMonitorViewController::setLoadState(LoadState s)
{
    if (mLoadState == s) return;
    mLoadState = s;
    emit loadStateChanged();
}

void CameraMonitorViewController::init()
{
    UIVIEW_LOG_DEBUG("CameraMonitorViewController::init");

    auto ctx = getAppContext();
    if (!ctx)
    {
        UIVIEW_LOG_WARN("no AppContext");
        setLoadState(Error);
        return;
    }

    mCameraDirectoryViewModel = ctx->getViewModelFactory()->createCameraDirectoryViewModelInstance();
    if (!mCameraDirectoryViewModel)
    {
        UIVIEW_LOG_ERROR("failed to create CameraDirectoryViewModel");
        setLoadState(Error);
        return;
    }

    // Create the tree model before QML binds to it.
    mCameraTreeModel = new CameraDirectoryItemModel(this);
    emit cameraTreeModelChanged();

    using Emitter = UIVMSignalEmitter::CameraDirectoryViewModelEmitter;
    auto* e = mCameraDirectoryEmitter.get();
    QObject::connect(e, &Emitter::signals_onCameraDirectoryReady,       this, &CameraMonitorViewController::onCameraDirectoryReady);
    QObject::connect(e, &Emitter::signals_onCameraDirectoryLoadFailed,  this, &CameraMonitorViewController::onCameraDirectoryLoadFailed);
    QObject::connect(e, &Emitter::signals_onCurrentCameraChanged,       this, &CameraMonitorViewController::onCurrentCameraChanged);
    QObject::connect(e, &Emitter::signals_onCameraGroupsAdded,       this, &CameraMonitorViewController::onCameraGroupsAdded);
    QObject::connect(e, &Emitter::signals_onCameraGroupsUpdated,     this, &CameraMonitorViewController::onCameraGroupsUpdated);
    QObject::connect(e, &Emitter::signals_onCameraGroupsRemoved,     this, &CameraMonitorViewController::onCameraGroupsRemoved);
    QObject::connect(e, &Emitter::signals_onCamerasAdded,            this, &CameraMonitorViewController::onCamerasAdded);
    QObject::connect(e, &Emitter::signals_onCamerasUpdated,          this, &CameraMonitorViewController::onCamerasUpdated);
    QObject::connect(e, &Emitter::signals_onCamerasRemoved,          this, &CameraMonitorViewController::onCamerasRemoved);
    QObject::connect(e, &Emitter::signals_onCameraRelationsAdded,    this, &CameraMonitorViewController::onCameraRelationsAdded);
    QObject::connect(e, &Emitter::signals_onCameraRelationsUpdated,  this, &CameraMonitorViewController::onCameraRelationsUpdated);
    QObject::connect(e, &Emitter::signals_onCameraRelationsRemoved,  this, &CameraMonitorViewController::onCameraRelationsRemoved);

    mCameraDirectoryViewModel->registerCallback(mCameraDirectoryEmitter);
    mCameraDirectoryViewModel->initViewModel();

}

void CameraMonitorViewController::selectNode(const QString& nodeId)
{
    if (!mCameraDirectoryViewModel)
    {
        return;
    }
    mCameraDirectoryViewModel->selectCamera(nodeId.toStdString());
}

bool CameraMonitorViewController::canDropOnNode(const QString& srcId, const QString& targetParentId) const
{
    if (!mCameraDirectoryViewModel)
    {
        return false;
    }
    return mCameraDirectoryViewModel->canMoveCameraNode(srcId.toStdString(), targetParentId.toStdString());
}

void CameraMonitorViewController::moveCameraNode(const QString& srcId, const QString& targetParentId)
{
    if (!mCameraDirectoryViewModel)
    {
        return;
    }
    mHasPendingMove    = true;
    mPendingMoveParent = targetParentId;
    mCameraDirectoryViewModel->moveCameraNode(srcId.toStdString(), targetParentId.toStdString());
}

QVariantMap CameraMonitorViewController::cameraSourceForNode(const QString& nodeId) const
{
    QVariantMap result;
    if (!mCameraDirectoryViewModel || nodeId.isEmpty())
    {
        return result;
    }
    auto srcOpt = mCameraDirectoryViewModel->getCameraSource(nodeId.toStdString());
    if (!srcOpt)
    {
        return result;
    }
    std::visit([&result](auto&& s) {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, commonHead::viewModels::model::LocalCameraSource>)
        {
            result.insert(QStringLiteral("kind"),  QStringLiteral("local"));
            result.insert(QStringLiteral("index"), s.index);
        }
        else if constexpr (std::is_same_v<T, commonHead::viewModels::model::NetworkCameraSource>)
        {
            result.insert(QStringLiteral("kind"),          QStringLiteral("network"));
            result.insert(QStringLiteral("url"),           QString::fromStdString(s.url));
            result.insert(QStringLiteral("transport"),     QString::fromStdString(s.transport));
            result.insert(QStringLiteral("openTimeoutMs"), s.openTimeoutMs);
            result.insert(QStringLiteral("readTimeoutMs"), s.readTimeoutMs);
        }
    }, *srcOpt);
    return result;
}

void CameraMonitorViewController::openCameraWindow(const QString& nodeId)
{
    if (!mCameraDirectoryViewModel || nodeId.isEmpty())
    {
        return;
    }
    auto srcOpt = mCameraDirectoryViewModel->getCameraSource(nodeId.toStdString());
    if (!srcOpt)
    {
        UIVIEW_LOG_WARN("openCameraWindow ignored: no source for nodeId=" << nodeId.toStdString());
        return;
    }
    auto ctx = getAppContext();
    if (!ctx)
    {
        UIVIEW_LOG_WARN("openCameraWindow: no AppContext");
        return;
    }
    auto win = ctx->getViewFactory()->createQmlItemWindow(
        QStringLiteral("UIView/MediaCameraView/qml/MediaCameraView.qml"));
    if (!win)
    {
        UIVIEW_LOG_WARN("openCameraWindow: failed to create window");
        return;
    }
    auto* mc = UIView::UIViewHelper::controllerOf<MediaCameraViewController>(win.data());
    if (!mc)
    {
        UIVIEW_LOG_WARN("openCameraWindow: window has no MediaCameraViewController");
        win->show();
        return;
    }
    mc->initializeController(ctx);
    std::visit([mc](auto&& s) {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, commonHead::viewModels::model::LocalCameraSource>)
        {
            mc->openLocalCamera(s.index);
        }
        else if constexpr (std::is_same_v<T, commonHead::viewModels::model::NetworkCameraSource>)
        {
            mc->openNetworkCamera(QString::fromStdString(s.url),
                                  QString::fromStdString(s.transport),
                                  s.openTimeoutMs,
                                  s.readTimeoutMs);
        }
    }, *srcOpt);
    UIView::UIViewHelper::centerOnParentWhenShown(win.data());
    win->show();
}

void CameraMonitorViewController::onCurrentCameraChanged(const QString& nodeId)
{
    QString displayName;
    if (!nodeId.isEmpty() && mCameraDirectoryViewModel)
    {
        if (auto tree = mCameraDirectoryViewModel->getCameraTree())
        {
            if (auto node = tree->findNodeById(nodeId.toStdString()))
            {
                displayName = QString::fromStdString(node->getNodeData().displayName);
            }
        }
    }
    if (mCurrentCameraId == nodeId && mCurrentCameraName == displayName)
    {
        return;
    }
    mCurrentCameraId   = nodeId;
    mCurrentCameraName = displayName;
    UIVIEW_LOG_DEBUG("onCurrentCameraChanged: " << nodeId.toStdString());
    emit currentCameraChanged();
}

// ---- Forwarding slots ----

void CameraMonitorViewController::onCameraDirectoryReady()
{
    UIVIEW_LOG_DEBUG("onCameraDirectoryReady received");
    if (mCameraTreeModel && mCameraDirectoryViewModel)
    {
        mCameraTreeModel->resetFromTree(mCameraDirectoryViewModel->getCameraTree());
    }
    setLoadState(Ready);
}

void CameraMonitorViewController::onCameraDirectoryLoadFailed(commonHead::viewModels::model::CameraDirectoryLoadError error)
{
    UIVIEW_LOG_ERROR("onCameraDirectoryLoadFailed received, error:" << static_cast<int>(error));
    setLoadState(Error);
}

void CameraMonitorViewController::onCameraGroupsAdded(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& v)
{ if (mCameraTreeModel) mCameraTreeModel->insertNodes(v); }

void CameraMonitorViewController::onCameraGroupsUpdated(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& v)
{ if (mCameraTreeModel) mCameraTreeModel->updateNodes(v); }

void CameraMonitorViewController::onCameraGroupsRemoved(const std::vector<std::string>& v)
{ if (mCameraTreeModel) mCameraTreeModel->removeNodes(v); }

void CameraMonitorViewController::onCamerasAdded(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& v)
{ if (mCameraTreeModel) mCameraTreeModel->insertNodes(v); }

void CameraMonitorViewController::onCamerasUpdated(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& v)
{ if (mCameraTreeModel) mCameraTreeModel->updateNodes(v); }

void CameraMonitorViewController::onCamerasRemoved(const std::vector<std::string>& v)
{ if (mCameraTreeModel) mCameraTreeModel->removeNodes(v); }

void CameraMonitorViewController::onCameraRelationsAdded(const std::vector<commonHead::viewModels::model::CameraDirectoryRelationData>& v)
{
    if (!mCameraTreeModel) return;
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(v.size());
    for (const auto& r : v) pairs.emplace_back(r.parentId, r.childId);
    mCameraTreeModel->setParents(pairs);
}

void CameraMonitorViewController::onCameraRelationsUpdated(const std::vector<commonHead::viewModels::model::CameraDirectoryRelationData>& v)
{
    if (!mCameraTreeModel) return;
    std::vector<std::pair<std::string, std::string>> pairs;
    pairs.reserve(v.size());
    for (const auto& r : v) pairs.emplace_back(r.parentId, r.childId);
    mCameraTreeModel->setParents(pairs);
    if (mHasPendingMove)
    {
        const QString p = mPendingMoveParent;
        mHasPendingMove = false;
        mPendingMoveParent.clear();
        emit nodeMoved(p);
    }
}

void CameraMonitorViewController::onCameraRelationsRemoved(const std::vector<std::string>& v)
{ if (mCameraTreeModel) mCameraTreeModel->clearParents(v); }
