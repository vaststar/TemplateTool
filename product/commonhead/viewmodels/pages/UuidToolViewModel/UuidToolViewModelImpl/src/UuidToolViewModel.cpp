#include "UuidToolViewModel.h"
#include "LoggerDefine.h"

#include <commonhead/viewmodels/UuidToolViewModel/UuidToolViewModelCreator.h>
#include <ucf/utilities/UUIDUtils/UUIDUtils.h>

namespace commonHead::viewModels {

std::shared_ptr<IUuidToolViewModel> impl::createUuidToolViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<UuidToolViewModel>(commonHeadFramework);
}

UuidToolViewModel::UuidToolViewModel(
    commonHead::ICommonHeadFrameworkWptr framework)
    : IUuidToolViewModel(framework)
{
    UUID_TOOL_VIEW_MODEL_LOG_DEBUG("UuidToolViewModel constructed, address: " << this);
}

UuidToolViewModel::~UuidToolViewModel()
{
    UUID_TOOL_VIEW_MODEL_LOG_DEBUG("UuidToolViewModel destroying, address: " << this);
}

std::string UuidToolViewModel::getViewModelName() const
{
    return "UuidToolViewModel";
}

void UuidToolViewModel::init()
{
}

std::string UuidToolViewModel::generate() const
{
    return ucf::utilities::UUIDUtils::generateUUID();
}

bool UuidToolViewModel::isValid(const std::string& uuid) const
{
    return ucf::utilities::UUIDUtils::isValidUUID(uuid);
}

} // namespace commonHead::viewModels
