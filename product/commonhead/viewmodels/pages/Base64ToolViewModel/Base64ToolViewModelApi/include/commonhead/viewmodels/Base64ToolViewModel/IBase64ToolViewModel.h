#pragma once

#include <string>

#include <commonhead/viewmodels/Base64ToolViewModel/Base64OperationTypes.h>
#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>

namespace commonHead::viewModels {

class IBase64ToolViewModel : public IViewModel
{
public:
    using IViewModel::IViewModel;
    IBase64ToolViewModel(const IBase64ToolViewModel&) = delete;
    IBase64ToolViewModel(IBase64ToolViewModel&&) = delete;
    IBase64ToolViewModel& operator=(const IBase64ToolViewModel&) = delete;
    IBase64ToolViewModel& operator=(IBase64ToolViewModel&&) = delete;
    ~IBase64ToolViewModel() override = default;

public:
    [[nodiscard]] virtual Base64Result encode(
        const std::string& input,
        bool urlSafe = false) const = 0;
    [[nodiscard]] virtual Base64Result decode(const std::string& input) const = 0;
};

} // namespace commonHead::viewModels
