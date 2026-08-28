#pragma once

#include <commonhead/viewmodels/Base64ToolViewModel/IBase64ToolViewModel.h>

namespace commonHead::viewModels {

class Base64ToolViewModel final : public IBase64ToolViewModel
{
public:
    explicit Base64ToolViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~Base64ToolViewModel() override = default;

    [[nodiscard]] std::string getViewModelName() const override;
    [[nodiscard]] Base64Result encode(
        const std::string& input,
        bool urlSafe = false) const override;
    [[nodiscard]] Base64Result decode(const std::string& input) const override;

protected:
    void init() override;
};

} // namespace commonHead::viewModels
