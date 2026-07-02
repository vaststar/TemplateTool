#include "MiniAppListViewModel.h"

#include <algorithm>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

namespace commonHead::viewModels{

std::shared_ptr<IMiniAppListViewModel> IMiniAppListViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MiniAppListViewModel>(commonHeadFramework);
}

MiniAppListViewModel::MiniAppListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMiniAppListViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create MiniAppListViewModel");

    // Shell stage: use placeholder data. Later this list will be discovered by
    // scanning the miniapps directory and parsing each manifest.json.
    mMiniApps =
    {
        { "com.example.notes",   "Notes",    "A simple note taking mini app", "" },
        { "com.example.crm",     "CRM",      "Customer relationship demo",    "" },
        { "com.example.report",  "Reports",  "Reporting dashboard demo",      "" },
    };
}

std::string MiniAppListViewModel::getViewModelName() const
{
    return "MiniAppListViewModel";
}

void MiniAppListViewModel::init()
{

}

std::vector<commonHead::viewModels::model::MiniAppInfo> MiniAppListViewModel::getMiniApps() const
{
    return mMiniApps;
}

commonHead::viewModels::model::MiniAppInfo MiniAppListViewModel::getMiniApp(const std::string& id) const
{
    const auto it = std::find_if(mMiniApps.begin(), mMiniApps.end(),
        [&id](const auto& app) { return app.id == id; });
    if (it != mMiniApps.end())
    {
        return *it;
    }
    return {};
}
}
