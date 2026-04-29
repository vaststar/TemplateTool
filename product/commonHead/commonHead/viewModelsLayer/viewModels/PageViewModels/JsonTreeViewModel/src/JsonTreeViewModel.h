#pragma once

#include <commonHead/viewModels/JsonTreeViewModel/IJsonTreeViewModel.h>

namespace commonHead::viewModels {

class JsonTreeViewModel : public IJsonTreeViewModel
{
public:
    explicit JsonTreeViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~JsonTreeViewModel() override = default;

    std::string getViewModelName() const override;

    model::JsonTreePtr parseJsonTree(const std::string& jsonStr) override;
    std::string getLastError() const override;

    std::string getJsonPath(const model::JsonTreePtr& tree, const model::IJsonTreeNode* node) const override;
    std::string getSubTreeJson(const model::JsonTreePtr& tree, const model::IJsonTreeNode* node, int indent = 2) const override;

protected:
    void init() override;

private:
    std::string m_lastError;
};

} // namespace commonHead::viewModels
