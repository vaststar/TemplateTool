#pragma once

#include <string>

#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>

namespace commonHead::viewModels {

class IUuidToolViewModel : public IViewModel
{
public:
    using IViewModel::IViewModel;
    IUuidToolViewModel(const IUuidToolViewModel&) = delete;
    IUuidToolViewModel(IUuidToolViewModel&&) = delete;
    IUuidToolViewModel& operator=(const IUuidToolViewModel&) = delete;
    IUuidToolViewModel& operator=(IUuidToolViewModel&&) = delete;
    ~IUuidToolViewModel() override = default;

public:
    [[nodiscard]] virtual std::string generate() const = 0;
    [[nodiscard]] virtual bool isValid(const std::string& uuid) const = 0;
};

} // namespace commonHead::viewModels
