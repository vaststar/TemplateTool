#include <UIUtilities/UIPlatformUtils.h>

#include <QDesktopServices>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QUrl>

#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include "LoggerDefine.h"

namespace UIUtilities{
void UIPlatformUtils::openLinkInDefaultBrowser(const std::string& url, const std::string& params)
{
    const QString target = QString::fromStdString(url);
#ifdef WIN32
    const auto wideTarget = target.toStdWString();
    const auto wideParams = QString::fromStdString(params).toStdWString();
    const auto result = reinterpret_cast<INT_PTR>(::ShellExecuteW(
        nullptr, L"open", wideTarget.c_str(), wideParams.c_str(), nullptr, SW_SHOW));
    if (result <= 32)
    {
        UIPlatformUtils_LOG_WARN("Could not request opening link, ShellExecute error: " << result);
    }
#elif defined(__APPLE__)
    Q_UNUSED(params);
    // Preserve open's target handling while passing the target without a shell.
    if (!QProcess::startDetached(QStringLiteral("/usr/bin/open"), {target}))
    {
        UIPlatformUtils_LOG_WARN("Could not start the link opener");
    }
#elif defined(__linux__)
    Q_UNUSED(params);
    if (!QDesktopServices::openUrl(QUrl(target)))
    {
        UIPlatformUtils_LOG_WARN("Could not request opening link");
    }
#else
    Q_UNUSED(target);
    Q_UNUSED(params);
#endif
}

void UIPlatformUtils::revealFileInFinder(const std::string& filePath)
{
    UIPlatformUtils_LOG_DEBUG("Revealing file in finder: " << filePath);
    if (filePath.empty())
    {
        UIPlatformUtils_LOG_WARN("Cannot reveal an empty file path");
        return;
    }

    const QString path = QString::fromStdString(filePath);
#ifdef WIN32
    const auto parameters = QStringLiteral("/select,\"%1\"").arg(path).toStdWString();
    const auto result = reinterpret_cast<INT_PTR>(::ShellExecuteW(
        nullptr, L"open", L"explorer.exe", parameters.c_str(), nullptr, SW_SHOW));
    if (result <= 32)
    {
        UIPlatformUtils_LOG_WARN("Could not request revealing file, ShellExecute error: " << result);
    }
#elif defined(__APPLE__)
    if (!QProcess::startDetached(QStringLiteral("/usr/bin/open"), {QStringLiteral("-R"), path}))
    {
        UIPlatformUtils_LOG_WARN("Could not start the file revealer");
    }
#elif defined(__linux__)
    // A desktop-independent fallback opens the parent directory without selecting the file.
    const QUrl parentDirectory = QUrl::fromLocalFile(QFileInfo(path).absolutePath());
    if (!QDesktopServices::openUrl(parentDirectory))
    {
        UIPlatformUtils_LOG_WARN("Could not request opening the file's parent directory");
    }
#else
    Q_UNUSED(path);
#endif
}
}
