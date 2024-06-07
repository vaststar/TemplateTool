#include "ContactService.h"
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

#include <sqlite3.h>
#include <curl/curl.h>

namespace ucf{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class ContactService::DataPrivate{
public:
    explicit DataPrivate(ICoreFrameworkWPtr coreFramework);
    std::weak_ptr<ICoreFramework> getCoreFramework() const;
    std::shared_ptr<model::ContactModel> getContactModel() const;
private:
    std::weak_ptr<ICoreFramework> mCoreFrameworkWPtr;
    std::shared_ptr<model::ContactModel>  mContactModelPtr;
};

ContactService::DataPrivate::DataPrivate(ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mContactModelPtr(std::make_shared<model::ContactModel>(coreFramework))
{

}

std::weak_ptr<ICoreFramework> ContactService::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

std::shared_ptr<model::ContactModel> ContactService::DataPrivate::getContactModel() const
{
    return mContactModelPtr;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ContactService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<IContactService> IContactService::CreateInstance(ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ContactService>(coreFramework);
}

ContactService::ContactService(ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreFramework))
{
    SERVICE_LOG_DEBUG("Create ContactService, address:" << this);
}

ContactService::~ContactService()
{
    SERVICE_LOG_DEBUG("Delete ContactService, address:" << this);
}

void ContactService::initService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }

    //
    sqlite3* pDb = NULL;
    sqlite3_initialize();
    sqlite3_open("test_tt.db", &pDb);
}

std::string ContactService::getServiceName() const
{
    return "ContactService";
}

void ContactService::OnServiceInitialized()
{
    SERVICE_LOG_DEBUG("");
}

void ContactService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("");
}

void ContactService::fetchContactList()
{
    SERVICE_LOG_DEBUG("start fetchContactList");
    std::vector<model::Contact> contactList;
    if (auto contactModel = mDataPrivate->getContactModel())
    {
        contactList = contactModel->getContacts();
    }
    fireNotification(&IContactServiceCallback::OnContactListAvailable, contactList);
    SERVICE_LOG_DEBUG("finish fetchContactList");
}

std::vector<model::Contact> ContactService::getContactList() const
{

    
CURL *curl;
    CURLcode res;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.example.com");
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        
        /* Check for errors */
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    if (auto contactModel = mDataPrivate->getContactModel())
    {
        return contactModel->getContacts();
    }

    return {};
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ContactService Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}