#pragma once
#include "AppRunner/ApplicationConfig.h"
#include "AppRunner/AppRunnerExport.h"
#include <memory>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkPtr = std::shared_ptr<ICommonHeadFramework>;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
}

namespace AppRunner
{
    struct FrameworkDependencies
    {
        ucf::framework::ICoreFrameworkPtr coreFramework;
        commonHead::ICommonHeadFrameworkPtr commonHeadFramework;
    };

    class APPRUNNER_EXPORT ApplicationRunner final
    {
    public:
        ApplicationRunner();
        ~ApplicationRunner();
    public:
        void initApp(int argc, char* argv[]);
        void exitApp();

        [[nodiscard]] commonHead::ICommonHeadFrameworkWPtr getCommonheadFramework() const;
        [[nodiscard]] ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    public:
        ApplicationRunner(const ApplicationRunner&) = delete;
        ApplicationRunner(ApplicationRunner&&) = delete;
        ApplicationRunner& operator=(const ApplicationRunner&) = delete;
        ApplicationRunner& operator=(ApplicationRunner&&) = delete;
    private:
        class DataPrivate;
        std::unique_ptr<DataPrivate> mDataPrivate;
    };
}
