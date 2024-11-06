#include "Adapters/ContactAdapter.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/NetworkService/Http/INetworkHttpManager.h>

#include <ucf/Services/NetworkService/Model/HttpRestRequest.h>
#include <ucf/Services/NetworkService/Model/HttpRestResponse.h>

#include <ucf/Services/NetworkService/Model/HttpRawRequest.h>
#include <ucf/Services/NetworkService/Model/HttpRawResponse.h>


#include <ucf/Services/NetworkService/Model/HttpDownloadToMemoryRequest.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToMemoryResponse.h>

#include <ucf/Services/NetworkService/Model/HttpDownloadToFileRequest.h>
#include <ucf/Services/NetworkService/Model/HttpDownloadToFileResponse.h>

#include "ContactServiceLogger.h"

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
    testFunc();
}

void ContactAdapter::testFunc()
{
    if (auto networkManager = mDataPrivate->getHttpManager().lock())
    {
        using namespace ucf::service::network::http; 
        //test post rest api
            ucf::service::network::http::HttpRestRequest postRestRequest{ucf::service::network::http::HTTPMethod::POST, "https://cisco.webex.com/wbxappapi/v1/meetingInfo", {{"Accept","application/json"}, {"Content-Type","application/json"}, {"Transfer-Encoding",""}}, R"({"sipUrl":"thzhu@cisco.webex.com"})", 30};
            auto postRestCallback = [](const ucf::service::network::http::HttpRestResponse& httpResponse){
                    SERVICE_LOG_DEBUG("postRestCallback body," << httpResponse.getResponseBody());
            };
            SERVICE_LOG_DEBUG("start post Rest");
            networkManager->sendHttpRestRequest(postRestRequest, postRestCallback);
        //test get rest api(auto redirect)
            ucf::service::network::http::HttpRestRequest getRestRequest{ucf::service::network::http::HTTPMethod::GET, "http://www.microsoft.com/", {{"Transfer-Encoding",""}},{}, 30};
            auto getRestCallback = [](const ucf::service::network::http::HttpRestResponse& httpResponse){
                    SERVICE_LOG_DEBUG("getRestCallback body," << httpResponse.getResponseBody());
            };
            SERVICE_LOG_DEBUG("start get Rest");
            networkManager->sendHttpRestRequest(getRestRequest, getRestCallback);

        // //test raw get request
        // ucf::service::network::http::HttpRawRequest rawGetRequest(ucf::service::network::http::HTTPMethod::GET, "http://www.microsoft.com/", {{"Transfer-Encoding",""}},{}, 30);
        // auto rawGetCallback = [](const ucf::service::network::http::HttpRawResponse& httpResponse) {
        //     auto body = httpResponse.getResponseBody();
        //     std::string pri = std::string{ body.begin(), body.end() };
        //     SERVICE_LOG_DEBUG("rawGetCallback body," << pri);
        // };
        //     SERVICE_LOG_DEBUG("start get Raw");
        // networkManager->sendHttpRawRequest(rawGetRequest, rawGetCallback);

        //test download content
        // HttpDownloadToMemoryRequest DownloadToMemoryRequest("https://ash-speed.hetzner.com/100MB.bin",{},30);
        // auto downloadMemoryCallBack = [](const HttpDownloadToMemoryResponse& downloadContent){
        //     SERVICE_LOG_DEBUG("download body, current:" << downloadContent.getResponseBody().size() << ", total:" << downloadContent.getTotalSize());
        // };
        //     SERVICE_LOG_DEBUG("start download to content Raw");
        // networkManager->downloadContentToMemory(DownloadToMemoryRequest, downloadMemoryCallBack);

        HttpDownloadToFileRequest downloadToFileRequest("https://ash-speed.hetzner.com/100MB.bin",{},3000,"scf_test_file");
        auto downloadFileCallBack = [](const HttpDownloadToFileResponse& downloadContent){
            SERVICE_LOG_DEBUG("download body, current:" << downloadContent.getCurrentSize() << ", total:" << downloadContent.getTotalSize());
        };
            SERVICE_LOG_DEBUG("start download to file");
        networkManager->downloadContentToFile(downloadToFileRequest, downloadFileCallBack);

    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish ContactAdapter Logic///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}