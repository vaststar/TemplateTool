#pragma once

#include <memory>
#include <string>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include "MockResourceLoader.h"

/**
 * Minimal framework facade: the real UIResourceLoaderManager only reaches the
 * data through getResourceLoader(), so this just exposes the MockResourceLoader.
 */
class MockCommonHeadFramework : public commonHead::ICommonHeadFramework
{
public:
    MockCommonHeadFramework()
        : mResourceLoader(std::make_shared<MockResourceLoader>()) {}

    std::shared_ptr<MockResourceLoader> mockLoader() const { return mResourceLoader; }

    std::string getName() const override { return "MockCommonHeadFramework"; }
    void initCommonheadFramework() override {}
    void exitCommonheadFramework() override {}

    commonHead::IResourceLoaderPtr getResourceLoader() const override { return mResourceLoader; }
    commonHead::IServiceLocatorPtr getServiceLocator() const override { return nullptr; }

private:
    std::shared_ptr<MockResourceLoader> mResourceLoader;
};
