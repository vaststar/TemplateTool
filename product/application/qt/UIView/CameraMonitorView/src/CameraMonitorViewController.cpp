#include "CameraMonitorView/CameraMonitorViewController.h"

#include <QUuid>

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

// ---- Context menu + CRUD ----

bool CameraMonitorViewController::canAddUnder(const QString& parentId, int nodeType) const
{
    if (!mCameraDirectoryViewModel)
    {
        return false;
    }
    using commonHead::viewModels::model::CameraDirectoryNodeType;
    const auto type = (nodeType == 1) ? CameraDirectoryNodeType::Camera : CameraDirectoryNodeType::Group;
    return mCameraDirectoryViewModel->canAddCameraNode(parentId.toStdString(), type);
}

bool CameraMonitorViewController::canRemove(const QString& nodeId) const
{
    if (!mCameraDirectoryViewModel)
    {
        return false;
    }
    return mCameraDirectoryViewModel->canRemoveCameraNode(nodeId.toStdString());
}

QString CameraMonitorViewController::nodeName(const QString& nodeId) const
{
    if (nodeId.isEmpty() || !mCameraDirectoryViewModel)
    {
        return {};
    }
    if (auto tree = mCameraDirectoryViewModel->getCameraTree())
    {
        if (auto node = tree->findNodeById(nodeId.toStdString()))
        {
            return QString::fromStdString(node->getNodeData().displayName);
        }
    }
    return {};
}

QVariantList CameraMonitorViewController::contextMenuModel(const QString& nodeId, int nodeType) const
{
    QVariantList items;
    const bool isRoot  = nodeId.isEmpty();
    const bool isGroup = (nodeType == 0);

    // New nodes may be created on blank space or under a group.
    if (isRoot || isGroup)
    {
        if (canAddUnder(nodeId, 0))
        {
            items.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("新增分组")},
                                     {QStringLiteral("action"), QStringLiteral("add_group")}});
        }
        if (canAddUnder(nodeId, 1))
        {
            items.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("新增摄像头")},
                                     {QStringLiteral("action"), QStringLiteral("add_camera")}});
        }
    }
    if (!isRoot)
    {
        if (!items.isEmpty())
        {
            items.append(QVariantMap{{QStringLiteral("separator"), true}});
        }
        items.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("重命名")},
                                 {QStringLiteral("action"), QStringLiteral("edit")}});
        if (canRemove(nodeId))
        {
            items.append(QVariantMap{{QStringLiteral("text"), QStringLiteral("删除")},
                                     {QStringLiteral("action"), QStringLiteral("delete")}});
        }
    }
    return items;
}

void CameraMonitorViewController::handleContextAction(const QString& action, const QString& nodeId, int nodeType)
{
    if (action == QLatin1String("add_group"))
    {
        openEditDialog(QStringLiteral("add_group"), nodeId, QString{}, 0, QString{});
    }
    else if (action == QLatin1String("add_camera"))
    {
        openEditDialog(QStringLiteral("add_camera"), nodeId, QString{}, 1, QString{});
    }
    else if (action == QLatin1String("edit"))
    {
        openEditDialog(QStringLiteral("edit"), QString{}, nodeId, nodeType, nodeName(nodeId));
    }
    else if (action == QLatin1String("delete"))
    {
        openDeleteDialog(nodeId);
    }
}

void CameraMonitorViewController::openEditDialog(const QString& mode, const QString& parentId,
                                                 const QString& editId, int nodeType, const QString& initialName)
{
    auto ctx = getAppContext();
    if (!ctx)
    {
        return;
    }
    auto win = ctx->getViewFactory()->createQmlWindow(
        QStringLiteral("UIView/CameraMonitorView/qml/CameraEditDialog.qml"),
        {
            { QStringLiteral("controller"),  QVariant::fromValue<QObject*>(this) },
            { QStringLiteral("mode"),        mode },
            { QStringLiteral("parentId"),    parentId },
            { QStringLiteral("editId"),      editId },
            { QStringLiteral("nodeType"),    nodeType },
            { QStringLiteral("initialName"), initialName },
        });
    if (!win)
    {
        return;
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}

void CameraMonitorViewController::openDeleteDialog(const QString& nodeId)
{
    auto ctx = getAppContext();
    if (!ctx)
    {
        return;
    }
    auto win = ctx->getViewFactory()->createQmlWindow(
        QStringLiteral("UIView/CameraMonitorView/qml/CameraDeleteDialog.qml"),
        {
            { QStringLiteral("controller"), QVariant::fromValue<QObject*>(this) },
            { QStringLiteral("targetId"),   nodeId },
        });
    if (!win)
    {
        return;
    }
    UIView::UIViewHelper::centerOnParentWhenShown(win);
    win->show();
}

void CameraMonitorViewController::addGroup(const QString& parentId, const QVariantMap& fields)
{
    if (!mCameraDirectoryViewModel)
    {
        return;
    }
    const QString name = fields.value(QStringLiteral("displayName")).toString().trimmed();
    if (name.isEmpty() || !canAddUnder(parentId, 0))
    {
        return;
    }
    const QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    mPendingAddId     = newId;
    mPendingAddParent = parentId;
    mCameraDirectoryViewModel->addCameraGroup(newId.toStdString(), name.toStdString());
    if (!parentId.isEmpty())
    {
        mCameraDirectoryViewModel->addRelation(parentId.toStdString(), newId.toStdString());
    }
}

void CameraMonitorViewController::addCamera(const QString& parentId, const QVariantMap& fields)
{
    if (!mCameraDirectoryViewModel)
    {
        return;
    }
    const QString name = fields.value(QStringLiteral("displayName")).toString().trimmed();
    if (name.isEmpty() || !canAddUnder(parentId, 1))
    {
        return;
    }

    // Build the camera source from the (deliberately minimal) dialog fields.
    using commonHead::viewModels::model::CameraSource;
    using commonHead::viewModels::model::LocalCameraSource;
    using commonHead::viewModels::model::NetworkCameraSource;
    CameraSource source;
    if (fields.value(QStringLiteral("sourceKind")).toString() == QLatin1String("network"))
    {
        NetworkCameraSource net;
        net.url = fields.value(QStringLiteral("url")).toString().trimmed().toStdString();
        source  = net;
    }
    else
    {
        LocalCameraSource local;
        local.index = fields.value(QStringLiteral("index"), 0).toInt();
        source      = local;
    }

    const QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    mPendingAddId     = newId;
    mPendingAddParent = parentId;
    mCameraDirectoryViewModel->addCamera(newId.toStdString(), name.toStdString(), source);
    if (!parentId.isEmpty())
    {
        mCameraDirectoryViewModel->addRelation(parentId.toStdString(), newId.toStdString());
    }
}

void CameraMonitorViewController::updateNode(const QString& nodeId, const QVariantMap& fields)
{
    if (!mCameraDirectoryViewModel || nodeId.isEmpty())
    {
        return;
    }
    const QString name = fields.value(QStringLiteral("displayName")).toString().trimmed();
    if (name.isEmpty())
    {
        return;
    }
    auto tree = mCameraDirectoryViewModel->getCameraTree();
    if (!tree)
    {
        return;
    }
    auto node = tree->findNodeById(nodeId.toStdString());
    if (!node)
    {
        return;
    }
    using commonHead::viewModels::model::CameraDirectoryNodeType;
    if (node->getNodeData().type == CameraDirectoryNodeType::Group)
    {
        mCameraDirectoryViewModel->updateCameraGroup(nodeId.toStdString(), name.toStdString());
    }
    else
    {
        // Rename only: keep the existing source intact.
        auto srcOpt = mCameraDirectoryViewModel->getCameraSource(nodeId.toStdString());
        if (!srcOpt)
        {
            return;
        }
        mCameraDirectoryViewModel->updateCamera(nodeId.toStdString(), name.toStdString(), *srcOpt);
    }
}

void CameraMonitorViewController::removeNode(const QString& nodeId)
{
    if (!mCameraDirectoryViewModel || nodeId.isEmpty())
    {
        return;
    }
    auto tree = mCameraDirectoryViewModel->getCameraTree();
    if (!tree)
    {
        return;
    }
    auto node = tree->findNodeById(nodeId.toStdString());
    if (!node)
    {
        return;
    }
    using commonHead::viewModels::model::CameraDirectoryNodeType;
    if (node->getNodeData().type == CameraDirectoryNodeType::Group)
    {
        mCameraDirectoryViewModel->removeCameraGroups({ nodeId.toStdString() });
    }
    else
    {
        mCameraDirectoryViewModel->removeCameras({ nodeId.toStdString() });
    }
}

void CameraMonitorViewController::maybeEmitNodeAdded(
    const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& v)
{
    // Root-level adds surface here (no relation row). Parented adds are handled in
    // onCameraRelationsAdded instead.
    if (mPendingAddId.isEmpty() || !mPendingAddParent.isEmpty())
    {
        return;
    }
    for (const auto& d : v)
    {
        if (QString::fromStdString(d.id) == mPendingAddId)
        {
            const QString id = mPendingAddId;
            mPendingAddId.clear();
            mPendingAddParent.clear();
            emit nodeAdded(id, QString{});
            break;
        }
    }
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
{ if (mCameraTreeModel) mCameraTreeModel->insertNodes(v); maybeEmitNodeAdded(v); }

void CameraMonitorViewController::onCameraGroupsUpdated(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& v)
{ if (mCameraTreeModel) mCameraTreeModel->updateNodes(v); }

void CameraMonitorViewController::onCameraGroupsRemoved(const std::vector<std::string>& v)
{ if (mCameraTreeModel) mCameraTreeModel->removeNodes(v); }

void CameraMonitorViewController::onCamerasAdded(const std::vector<commonHead::viewModels::model::CameraDirectoryNodeData>& v)
{ if (mCameraTreeModel) mCameraTreeModel->insertNodes(v); maybeEmitNodeAdded(v); }

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
    // Parented adds materialise once the relation row arrives.
    if (!mPendingAddId.isEmpty() && !mPendingAddParent.isEmpty())
    {
        for (const auto& r : v)
        {
            if (QString::fromStdString(r.childId) == mPendingAddId)
            {
                const QString id     = mPendingAddId;
                const QString parent = mPendingAddParent;
                mPendingAddId.clear();
                mPendingAddParent.clear();
                emit nodeAdded(id, parent);
                break;
            }
        }
    }
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
