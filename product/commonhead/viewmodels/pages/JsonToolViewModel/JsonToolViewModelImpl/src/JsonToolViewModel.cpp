#include "JsonToolViewModel.h"
#include "JsonTreeModel.h"

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ResourceLoader/IResourceLoader.h>
#include <commonhead/ResourceStringLoader/ResourceString.h>
#include <commonhead/viewmodels/JsonToolViewModel/JsonToolViewModelCreator.h>
#include <ucf/utilities/JsonUtils/JsonValue.h>

namespace commonHead::viewModels {

namespace impl {

std::shared_ptr<IJsonToolViewModel> createJsonToolViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<JsonToolViewModel>(commonHeadFramework);
}

} // namespace impl

JsonToolViewModel::JsonToolViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : IJsonToolViewModel(framework)
{
}

void JsonToolViewModel::init()
{
    // No initialization needed — stateless parse on demand
}

std::string JsonToolViewModel::getViewModelName() const
{
    return "JsonToolViewModel";
}

JsonFormatResult JsonToolViewModel::format(
    const std::string& input,
    int indent) const
{
    const auto parsed = ucf::utilities::JsonValue::parseEx(input);
    if (!parsed.ok())
    {
        return JsonFormatResult{
            .success = false,
            .data = {},
            .errorMessage = parsed.error};
    }

    return JsonFormatResult{
        .success = true,
        .data = parsed.value.dumpPretty(indent),
        .errorMessage = {}};
}

JsonFormatResult JsonToolViewModel::minify(const std::string& input) const
{
    const auto parsed = ucf::utilities::JsonValue::parseEx(input);
    if (!parsed.ok())
    {
        return JsonFormatResult{
            .success = false,
            .data = {},
            .errorMessage = parsed.error};
    }

    return JsonFormatResult{
        .success = true,
        .data = parsed.value.dump(),
        .errorMessage = {}};
}

JsonFormatResult JsonToolViewModel::validate(const std::string& input) const
{
    const auto parsed = ucf::utilities::JsonValue::parseEx(input);
    if (!parsed.ok())
    {
        return JsonFormatResult{
            .success = false,
            .data = {},
            .errorMessage = parsed.error};
    }

    auto resourceLoader = lockResourceLoader();
    if (!resourceLoader)
    {
        return JsonFormatResult{
            .success = false,
            .data = {},
            .errorMessage = "ResourceLoader is not available"};
    }

    return JsonFormatResult{
        .success = true,
        .data = resourceLoader->getLocalizedString(
            commonHead::model::LocalizedString::ToolsJsonValid),
        .errorMessage = {}};
}

model::JsonTreePtr JsonToolViewModel::parseJsonTree(const std::string& jsonStr)
{
    m_lastError.clear();
    auto tree = model::JsonTree::build(jsonStr, m_lastError);
    return tree;
}

std::string JsonToolViewModel::getLastError() const
{
    return m_lastError;
}

std::string JsonToolViewModel::getJsonPath(
    const model::JsonTreePtr& tree,
    const model::IJsonTreeNode* node) const
{
    if (!tree)
    {
        return "$";
    }
    return tree->getJsonPath(node);
}

std::string JsonToolViewModel::getSubTreeJson(
    const model::JsonTreePtr& tree,
    const model::IJsonTreeNode* node,
    int indent) const
{
    if (!tree)
    {
        return {};
    }
    return tree->getSubTreeJson(node, indent);
}

std::shared_ptr<commonHead::IResourceLoader> JsonToolViewModel::lockResourceLoader() const
{
    auto framework = getCommonHeadFramework().lock();
    if (!framework)
    {
        return {};
    }
    return framework->getResourceLoader();
}

} // namespace commonHead::viewModels
