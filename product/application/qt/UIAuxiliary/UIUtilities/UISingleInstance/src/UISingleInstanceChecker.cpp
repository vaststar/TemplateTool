#include <UISingleInstance/UISingleInstanceChecker.h>

#include <QLockFile>
#include <QDir>


namespace UIUtilities{
struct SingleInstanceChecker::Impl
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


// ---------------- SingleInstanceChecker 接口实现 ----------------

SingleInstanceChecker::SingleInstanceChecker(const std::string &name)
    : pImpl(std::make_unique<Impl>(QString::fromStdString(name)))
{
}

SingleInstanceChecker::~SingleInstanceChecker() = default;


bool SingleInstanceChecker::tryToRun()
{
    return pImpl->tryToRun();
}
}
