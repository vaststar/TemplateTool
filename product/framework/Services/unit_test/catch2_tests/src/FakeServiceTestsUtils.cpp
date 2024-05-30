
#include "ServicesTests/FakeServiceTestsUtils.h"


namespace FakeServiceTestsUtils
{
    std::shared_ptr<ucf::ICoreFramework> buildFakeCoreFramework()
    {
        return ucf::ICoreFramework::CreateInstance();
    }
} // namespace FakeServiceTestsUtils
