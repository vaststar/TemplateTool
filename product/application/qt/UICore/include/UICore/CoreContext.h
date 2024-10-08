#pragma once

#include <memory>
#include <QObject>

#include <UICore/UICoreExport.h>

class  CoreViewFactory;
class CoreViewModelFactory;

class UICore_EXPORT CoreContext final: public QObject
{
Q_OBJECT
public:
    CoreContext(std::unique_ptr<CoreViewModelFactory>&& viewModelFactory, std::unique_ptr<CoreViewFactory>&& viewFactory);
    ~CoreContext();
    
    CoreContext(const CoreContext&) = delete;
    CoreContext(CoreContext&&) = delete;
    CoreContext& operator=(const CoreContext&) = delete;
    CoreContext& operator=(CoreContext&&) = delete;

    const std::unique_ptr<CoreViewFactory>& getViewFactory() const;
    const std::unique_ptr<CoreViewModelFactory>& getViewModelFactory() const;
private:
    const std::unique_ptr<CoreViewFactory> mViewFactory;
    const std::unique_ptr<CoreViewModelFactory> mViewModelFactory;
};