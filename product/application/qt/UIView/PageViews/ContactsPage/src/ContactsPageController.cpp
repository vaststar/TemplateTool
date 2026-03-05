#include "PageViews/ContactsPage/include/ContactsPageController.h"
#include "LoggerDefine/LoggerDefine.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>
#include <AppContext/AppContext.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>

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
    mOrgTreeModel->setUpViewModel(mContactListViewModel);
    emit orgTreeModelChanged();
}
