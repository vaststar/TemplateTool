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
QString CameraMonitorViewController::getSelectedCameraId() const             { return mSelectedCameraId; }
QString CameraMonitorViewController::getSelectedCameraName() const           { return mSelectedCameraName; }
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
        setLoadState(Error);
        return;
    }

    using Emitter = UIVMSignalEmitter::CameraDirectoryViewModelEmitter;
    auto* e = mCameraDirectoryEmitter.get();
    QObject::connect(e, &Emitter::signals_onCameraDirectoryReady,    this, &CameraMonitorViewController::onCameraDirectoryReady);
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

    mCameraTreeModel = new CameraDirectoryItemModel(this);
    emit cameraTreeModelChanged();

    if (auto tree = mCameraDirectoryViewModel->getCameraTree())
    {
        mCameraTreeModel->resetFromTree(tree);
        setLoadState(Ready);
    }
}

void CameraMonitorViewController::selectNode(const QString& nodeId)
{
    if (nodeId.isEmpty() || !mCameraDirectoryViewModel)
    {
        return;
    }
    auto source = mCameraDirectoryViewModel->getCameraSource(nodeId.toStdString());
    if (!source)
    {
        UIVIEW_LOG_DEBUG("selectNode ignored (not a camera): " << nodeId.toStdString());
        return;
    }
    if (mSelectedCameraId == nodeId)
    {
        return;
    }

    QString displayName;
    if (auto tree = mCameraDirectoryViewModel->getCameraTree())
    {
        if (auto node = tree->findNodeById(nodeId.toStdString()))
        {
            displayName = QString::fromStdString(node->getNodeData().displayName);
        }
    }
    mSelectedCameraId   = nodeId;
    mSelectedCameraName = displayName;
    emit selectedCameraChanged();
    UIVIEW_LOG_DEBUG("selectNode: " << nodeId.toStdString());
}

// ---- Thin forwarding slots ----

void CameraMonitorViewController::onCameraDirectoryReady()
{
    if (mCameraTreeModel && mCameraDirectoryViewModel)
    {
        mCameraTreeModel->resetFromTree(mCameraDirectoryViewModel->getCameraTree());
    }
    setLoadState(Ready);
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
