#include "ContactService/ContactService.h"
#include "CoreFramework/ICoreFramework.h"
#include "ServiceCommonFile/ServiceLogger.h"

#include <sqlite3.h>
#include <curl/curl.h>

std::shared_ptr<IContactService> IContactService::CreateInstance(ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<ContactService>(coreFramework);
}

ContactService::ContactService(ICoreFrameworkWPtr coreFramework)
    :mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("Create ContactService, address:" << this);
}

void ContactService::initService()
{
    mContactModelPtr = std::make_unique<model::ContactModel>(mCoreFrameworkWPtr);
    if (auto coreFramework = mCoreFrameworkWPtr.lock())
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

void ContactService::OnDataBaseInitialized()
{

}

void ContactService::OnServiceInitialized()
{
    SERVICE_LOG_DEBUG("");
}

void ContactService::fetchContactList()
{
    SERVICE_LOG_DEBUG("start fetchContactList");
    std::vector<model::Contact> contactList;
    if (mContactModelPtr)
    {
        contactList = mContactModelPtr->getContacts();
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


    if (mContactModelPtr)
    {
        return mContactModelPtr->getContacts();
    }















    return {};
}