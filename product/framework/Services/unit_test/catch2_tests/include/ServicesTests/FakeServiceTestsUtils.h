#pragma once
#include <ucf/CoreFramework/ICoreFramework.h>
#include <memory>
namespace FakeServiceTestsUtils
{
    std::shared_ptr<ucf::framework::ICoreFramework> buildFakeCoreFramework();
} // namespace FakeServiceTestsUtils
