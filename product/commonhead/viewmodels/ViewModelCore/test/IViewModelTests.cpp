#include <catch2/catch_test_macros.hpp>

#include <string>

#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>

namespace {

class CountingViewModel final : public commonHead::viewModels::IViewModel
{
public:
    CountingViewModel()
        : IViewModel({})
    {
    }

    std::string getViewModelName() const override
    {
        return "CountingViewModel";
    }

    int getInitCount() const
    {
        return mInitCount;
    }

protected:
    void init() override
    {
        ++mInitCount;
    }

private:
    int mInitCount{0};
};

} // namespace

TEST_CASE("IViewModel initializes only once", "[ViewModelCore]")
{
    CountingViewModel viewModel;

    viewModel.initViewModel();
    viewModel.initViewModel();

    REQUIRE(viewModel.getInitCount() == 1);
}
