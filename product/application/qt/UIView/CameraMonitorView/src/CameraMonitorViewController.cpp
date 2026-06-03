#include "CameraMonitorView/CameraMonitorViewController.h"

#include <commonHead/viewModels/CameraDirectoryViewModel/ICameraDirectoryViewModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"
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

    // Create the tree model first so QML can bind to it while we are still loading.
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

    // Register the emitter BEFORE probing isCameraDirectoryReady(): if the VM transitions
    // between the probe and the registration we would otherwise miss the Ready / change
    // events that arrive in that window.
    mCameraDirectoryViewModel->registerCallback(mCameraDirectoryEmitter);
    mCameraDirectoryViewModel->initViewModel();

    // if (mCameraDirectoryViewModel->isCameraDirectoryReady())
    // {
    //     // VM already loaded (we joined late); pull the snapshot synchronously.
    //     UIVIEW_LOG_DEBUG("CameraDirectoryViewModel already ready on init, pulling snapshot");
    //     mCameraTreeModel->resetFromTree(mCameraDirectoryViewModel->getCameraTree());
    //     setLoadState(Ready);
    // }
    // else
    // {
    //     // Wait for onCameraDirectoryReady / onCameraDirectoryLoadFailed.
    //     UIVIEW_LOG_DEBUG("CameraDirectoryViewModel not ready yet, waiting for load notification");
    //     setLoadState(Loading);
    // }

    // // Pull initial selection in case a sibling VM consumer already set one before we joined.
    // if (const auto initialId = mCameraDirectoryViewModel->getCurrentCameraId(); !initialId.empty())
    // {
    //     onCurrentCameraChanged(QString::fromStdString(initialId));
    // }
}

void CameraMonitorViewController::selectNode(const QString& nodeId)
{
    if (!mCameraDirectoryViewModel)
    {
        return;
    }
    // Pure forward: VM owns the selection state. The mirror is updated when the VM fires
    // onCurrentCameraChanged back to us. The VM also validates that the id refers to a
    // real camera and dedupes against the current value.
    mCameraDirectoryViewModel->selectCamera(nodeId.toStdString());
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

// ---- Thin forwarding slots ----

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
}

void CameraMonitorViewController::onCameraRelationsRemoved(const std::vector<std::string>& v)
{ if (mCameraTreeModel) mCameraTreeModel->clearParents(v); }
