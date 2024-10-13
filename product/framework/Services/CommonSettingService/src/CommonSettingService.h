#pragma once

#include <memory>
#include <ucf/Services/CommonSettingService/ICommonSettingService.h>

namespace ucf::service{
class CommonSettingService final: public ICommonSettingService
{
public:
    explicit CommonSettingService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~CommonSettingService();
    CommonSettingService(const CommonSettingService&) = delete;
    CommonSettingService(CommonSettingService&&) = delete;
    CommonSettingService& operator=(const CommonSettingService&) = delete;
    CommonSettingService& operator=(CommonSettingService&&) = delete;

    //IService
    virtual std::string getServiceName() const override;
    virtual void initService() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}