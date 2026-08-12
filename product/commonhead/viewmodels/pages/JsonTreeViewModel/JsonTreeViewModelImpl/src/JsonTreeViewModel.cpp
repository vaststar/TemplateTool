#include "JsonTreeViewModel.h"
#include "JsonTreeModel.h"

#include <commonhead/viewmodels/JsonTreeViewModel/JsonTreeViewModelCreator.h>

namespace commonHead::viewModels {

namespace impl {

std::shared_ptr<IJsonTreeViewModel> createJsonTreeViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<JsonTreeViewModel>(commonHeadFramework);
}

} // namespace impl

JsonTreeViewModel::JsonTreeViewModel(commonHead::ICommonHeadFrameworkWptr framework)
    : IJsonTreeViewModel(framework)
{
}

void JsonTreeViewModel::init()
{
    // No initialization needed — stateless parse on demand
}

std::string JsonTreeViewModel::getViewModelName() const
{
    return "JsonTreeViewModel";
}

model::JsonTreePtr JsonTreeViewModel::parseJsonTree(const std::string& jsonStr)
{
    m_lastError.clear();
    auto tree = model::JsonTree::build(jsonStr, m_lastError);
    return tree;
}

std::string JsonTreeViewModel::getLastError() const
{
    return m_lastError;
}

std::string JsonTreeViewModel::getJsonPath(
    const model::JsonTreePtr& tree,
    const model::IJsonTreeNode* node) const
{
    if (!tree)
    {
        return "$";
    }
    return tree->getJsonPath(node);
}

std::string JsonTreeViewModel::getSubTreeJson(
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

} // namespace commonHead::viewModels
