
#include "ServicesTests/FakeServiceTestsUtils.h"


namespace FakeServiceTestsUtils
{
    std::shared_ptr<ucf::framework::ICoreFramework> buildFakeCoreFramework()
    {
        return ucf::framework::ICoreFramework::createInstance();
    }
} // namespace FakeServiceTestsUtils
