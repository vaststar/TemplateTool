
#include "ServicesTests/FakeServiceTestsUtils.h"


namespace FakeServiceTestsUtils
{
    std::shared_ptr<ICoreFramework> buildFakeCoreFramework()
    {
        return ICoreFramework::CreateInstance();
    }
} // namespace FakeServiceTestsUtils
