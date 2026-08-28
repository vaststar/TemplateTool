#include "Base64ToolViewModel.h"
#include "LoggerDefine.h"

#include <commonhead/viewmodels/Base64ToolViewModel/Base64ToolViewModelCreator.h>
#include <ucf/utilities/Base64Utils/Base64Utils.h>

namespace commonHead::viewModels {

std::shared_ptr<IBase64ToolViewModel> impl::createBase64ToolViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<Base64ToolViewModel>(commonHeadFramework);
}

Base64ToolViewModel::Base64ToolViewModel(
    commonHead::ICommonHeadFrameworkWptr framework)
    : IBase64ToolViewModel(framework)
{
    BASE64_TOOL_VIEW_MODEL_LOG_DEBUG("Base64ToolViewModel constructed, address: " << this);
}

Base64ToolViewModel::~Base64ToolViewModel()
{
    BASE64_TOOL_VIEW_MODEL_LOG_DEBUG("Base64ToolViewModel destroying, address: " << this);
}

std::string Base64ToolViewModel::getViewModelName() const
{
    return "Base64ToolViewModel";
}

void Base64ToolViewModel::init()
{
}

Base64Result Base64ToolViewModel::encode(
    const std::string& input,
    bool urlSafe) const
{
    const auto variant = urlSafe
        ? ucf::utilities::Base64Variant::UrlSafe
        : ucf::utilities::Base64Variant::Standard;
    const auto encoded = ucf::utilities::Base64Utils::encode(input, variant);

    return Base64Result{
        .success = encoded.success,
        .data = encoded.data,
        .errorMessage = encoded.errorMessage};
}

Base64Result Base64ToolViewModel::decode(const std::string& input) const
{
    const auto decoded = ucf::utilities::Base64Utils::decode(input);

    Base64Result result;
    result.success = decoded.success;
    if (decoded.success)
    {
        result.data.assign(decoded.data.begin(), decoded.data.end());
    }
    result.errorMessage = decoded.errorMessage;
    return result;
}

} // namespace commonHead::viewModels
