#include "Adapters/ContactAdapter.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/NetworkService/Http/INetworkHttpManager.h>

#include <ucf/Services/NetworkService/Model/HttpRestRequest.h>
#include <ucf/Services/NetworkService/Model/HttpRestResponse.h>

namespace ucf::adapter{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

using INetworkHttpManagerWPtr = std::weak_ptr<ucf::service::network::http::INetworkHttpManager>;
class ContactAdapter::DataPrivate{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    INetworkHttpManagerWPtr getHttpManager() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
};

ContactAdapter::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
{

}

ucf::framework::ICoreFrameworkWPtr ContactAdapter::DataPrivate::getCoreFramework() const
{
    return mCoreFrameworkWPtr;
}

INetworkHttpManagerWPtr ContactAdapter::DataPrivate::getHttpManager() const
{
    if (auto coreframework = getCoreFramework().lock())
    {
        if (auto networkService = coreframework->getService<ucf::service::INetworkService>().lock())
        {
            return networkService->getNetworkHttpManager();
        }
    }
    return {};
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ContactAdapter Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
ContactAdapter::ContactAdapter(ucf::framework::ICoreFrameworkWPtr coreframework)
    : mDataPrivate(std::make_unique<DataPrivate>(coreframework))
{
    SERVICE_LOG_DEBUG("Create ContactAdapter, address:" << this);
}

ContactAdapter::~ContactAdapter()
{
    SERVICE_LOG_DEBUG("Delete ContactAdapter, address:" << this);
}

void ContactAdapter::fetchContactInfo(const std::string& contactId, fetchContactInfoCallBack)
{
    if (auto networkManager = mDataPrivate->getHttpManager().lock())
    {
        // auto httpRequest = ucf::utilities::network::http::NetworkHttpRequest(ucf::utilities::network::http::HTTPMethod::POST, "https://cisco.webex.com/wbxappapi/v1/meetingInfo", {{"Accept","application/json"}, {"Content-Type","application/json"}, {"Transfer-Encoding",""}}, 30, R"({"sipUrl":"thzhu@cisco.webex.com"})");
        // auto httpRequest = ucf::utilities::network::http::NetworkHttpRequest(ucf::utilities::network::http::HTTPMethod::GET, "http://www.microsoft.com/", {{"Transfer-Encoding",""}}, 30);
    
        // auto httpRequest = ucf::service::network::http::NetworkHttpRequest(ucf::service::network::http::HTTPMethod::GET, "https://www.dundeecity.gov.uk/sites/default/files/publications/civic_renewal_forms.zip", {{"Transfer-Encoding",""}}, 30);
        
        // auto httpResponseCallback = [](const ucf::utilities::network::http::NetworkHttpResponse& response){
            
        //     SERVICE_LOG_DEBUG("got response, body:" << response.getResponseBody());
        // };

        auto callback = [](const ucf::service::network::http::HttpRestResponse& httpResponse){};
        ucf::service::network::http::HttpRestRequest httpRequest{ucf::service::network::http::HTTPMethod::POST, "https://cisco.webex.com/wbxappapi/v1/meetingInfo", {{"Accept","application/json"}, {"Content-Type","application/json"}, {"Transfer-Encoding",""}}, R"({"sipUrl":"thzhu@cisco.webex.com"})", 30};
        networkManager->sendHttpRestRequest(httpRequest, callback);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ContactAdapter Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}