#include <UISingleInstance/UISingleInstanceChecker.h>

#include <QLockFile>
#include <QDir>


namespace UIUtilities{
struct UISingleInstanceChecker::Impl
{
public:
    explicit Impl(const QString &name)
        : lockFile(QDir::temp().absoluteFilePath(name + ".lock"))
    {
        lockFile.setStaleLockTime(0);
    }

    bool tryToRun()
    {
        return lockFile.tryLock();
    }
private:
    QLockFile lockFile;
};


// ---------------- UISingleInstanceChecker 接口实现 ----------------

UISingleInstanceChecker::UISingleInstanceChecker(const std::string &name)
    : pImpl(std::make_unique<Impl>(QString::fromStdString(name)))
{
}

UISingleInstanceChecker::~UISingleInstanceChecker() = default;


bool UISingleInstanceChecker::tryToRun()
{
    return pImpl->tryToRun();
}
}
