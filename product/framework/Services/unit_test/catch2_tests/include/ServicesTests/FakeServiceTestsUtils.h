#pragma once
#include "CoreFramework/ICoreFramework.h"
#include <memory>
namespace FakeServiceTestsUtils
{
    std::shared_ptr<ICoreFramework> buildFakeCoreFramework();
} // namespace FakeServiceTestsUtils
