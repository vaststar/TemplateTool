#pragma once

#include <memory>

#include <commonhead/viewmodels/JsonToolViewModel/IJsonToolViewModel.h>

namespace commonHead {
class IResourceLoader;
}

namespace commonHead::viewModels {

class JsonToolViewModel final : public IJsonToolViewModel
{
public:
    explicit JsonToolViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~JsonToolViewModel() override = default;

    std::string getViewModelName() const override;

    [[nodiscard]] JsonFormatResult format(
        const std::string& input,
        int indent = 2) const override;
    [[nodiscard]] JsonFormatResult minify(const std::string& input) const override;
    [[nodiscard]] JsonFormatResult validate(const std::string& input) const override;

    model::JsonTreePtr parseJsonTree(const std::string& jsonStr) override;
    std::string getLastError() const override;

    std::string getJsonPath(
        const model::JsonTreePtr& tree,
        const model::IJsonTreeNode* node) const override;
    std::string getSubTreeJson(
        const model::JsonTreePtr& tree,
        const model::IJsonTreeNode* node,
        int indent = 2) const override;

protected:
    void init() override;

private:
    [[nodiscard]] std::shared_ptr<commonHead::IResourceLoader> lockResourceLoader() const;

private:
    std::string m_lastError;
};

} // namespace commonHead::viewModels
