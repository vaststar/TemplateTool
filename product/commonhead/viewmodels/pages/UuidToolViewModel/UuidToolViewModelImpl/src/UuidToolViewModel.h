#pragma once

#include <commonhead/viewmodels/UuidToolViewModel/IUuidToolViewModel.h>

namespace commonHead::viewModels {

class UuidToolViewModel final : public IUuidToolViewModel
{
public:
    explicit UuidToolViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~UuidToolViewModel() override;

    [[nodiscard]] std::string getViewModelName() const override;
    [[nodiscard]] std::string generate() const override;
    [[nodiscard]] bool isValid(const std::string& uuid) const override;

protected:
    void init() override;
};

} // namespace commonHead::viewModels
