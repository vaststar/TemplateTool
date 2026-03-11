#include "PageViews/ContactsPage/include/ContactsPageController.h"
#include "LoggerDefine/LoggerDefine.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>
#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <QVariantMap>

ContactsPageController::ContactsPageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create ContactsPageController");
}

void ContactsPageController::init()
{
    UIVIEW_LOG_DEBUG("ContactsPageController::init");
    mContactListViewModel = getAppContext()->getViewModelFactory()->createContactListViewModelInstance();
    mContactListViewModel->initViewModel();
    buildContactTreeModel();
    UIVIEW_LOG_DEBUG("ContactsPageController::init done");
}

void ContactsPageController::buttonClicked()
{
    getAppContext()->getViewFactory()->loadQmlWindow(QStringLiteral("UTComponent/UTWindow/UTWindow.qml"));
}

QAbstractItemModel* ContactsPageController::getOrgTreeModel() const
{
    return mOrgTreeModel;
}

QString ContactsPageController::getCurrentContactId() const
{
    return m_currentContactId;
}

void ContactsPageController::selectContact(const QString& contactId)
{
    if (m_currentContactId == contactId)
        return;
    m_currentContactId = contactId;
    emit currentContactIdChanged();
    UIVIEW_LOG_DEBUG("selectContact: " << contactId.toStdString());
}

QVariantMap ContactsPageController::getContactInfo(const QString& contactId) const
{
    QVariantMap result;
    if (contactId.isEmpty() || !mContactListViewModel)
        return result;
    
    auto tree = mContactListViewModel->getContactList();
    if (!tree)
        return result;
        
    auto node = tree->findNodeById(contactId.toStdString());
    if (node) {
        auto data = node->getNodeData();
        result["id"] = QString::fromStdString(data.id);
        result["name"] = QString::fromStdString(data.displayName);
        result["type"] = static_cast<int>(data.type);
    }
    return result;
}

void ContactsPageController::buildContactTreeModel()
{
    if (!mContactListViewModel)
    {
        UIVIEW_LOG_ERROR("mContactListViewModel is null");
        return;
    }

    if (mOrgTreeModel)
    {
        delete mOrgTreeModel;
        mOrgTreeModel = nullptr;
    }

    mOrgTreeModel = new ContactListItemModel(this);
    mOrgTreeModel->setTree(mContactListViewModel->getContactList());
    emit orgTreeModelChanged();
}
