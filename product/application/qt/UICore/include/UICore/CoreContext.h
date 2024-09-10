#pragma once

#include <memory>

class  CoreViewFactory;
#include <UICore/UICoreExport.h>
class UICore_EXPORT CoreContext final
{
public:
    CoreContext(std::unique_ptr<CoreViewFactory>&& viewFactory);
    ~CoreContext();
    
    CoreContext(const CoreContext&) = delete;
    CoreContext(CoreContext&&) = delete;
    CoreContext& operator=(const CoreContext&) = delete;
    CoreContext& operator=(CoreContext&&) = delete;

    const std::unique_ptr<CoreViewFactory>& getViewFactory() const;
private:
    const std::unique_ptr<CoreViewFactory> mViewFactory;
    
};