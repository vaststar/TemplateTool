#include "AppSchemeHandler.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>

namespace MiniAppRuntime {

namespace {

QString mimeTypeForSuffix(const QString& path)
{
    // Fast path for common web assets; fall back to QMimeDatabase.
    const QString lower = path.toLower();
    if (lower.endsWith(QStringLiteral(".html")) || lower.endsWith(QStringLiteral(".htm")))
        return QStringLiteral("text/html");
    if (lower.endsWith(QStringLiteral(".js")) || lower.endsWith(QStringLiteral(".mjs")))
        return QStringLiteral("text/javascript");
    if (lower.endsWith(QStringLiteral(".css")))
        return QStringLiteral("text/css");
    if (lower.endsWith(QStringLiteral(".json")))
        return QStringLiteral("application/json");
    if (lower.endsWith(QStringLiteral(".svg")))
        return QStringLiteral("image/svg+xml");
    if (lower.endsWith(QStringLiteral(".png")))
        return QStringLiteral("image/png");
    if (lower.endsWith(QStringLiteral(".jpg")) || lower.endsWith(QStringLiteral(".jpeg")))
        return QStringLiteral("image/jpeg");
    if (lower.endsWith(QStringLiteral(".woff2")))
        return QStringLiteral("font/woff2");

    const QMimeDatabase db;
    const QString type = db.mimeTypeForFile(path).name();
    return type.isEmpty() ? QStringLiteral("application/octet-stream") : type;
}

} // namespace

AppSchemeHandler::AppSchemeHandler(const QString& packageDir)
{
    const QFileInfo info(packageDir);
    if (info.exists() && info.isDir())
    {
        m_packageRoot = info.canonicalFilePath();
    }
}

AppSchemeHandler::Response AppSchemeHandler::resolve(const QUrl& url) const
{
    Response response;
    if (m_packageRoot.isEmpty())
    {
        return response;
    }

    // Path after the host is package-relative; a bare "app://<id>/" is rejected.
    QString relative = url.path();
    while (relative.startsWith(QLatin1Char('/')))
    {
        relative.remove(0, 1);
    }
    if (relative.isEmpty())
    {
        return response;
    }

    const QString candidate = QDir(m_packageRoot).filePath(relative);
    const QFileInfo fileInfo(candidate);
    if (!fileInfo.exists() || !fileInfo.isFile())
    {
        return response;
    }

    // Confine to the package root: defeats "../" traversal and symlink escapes.
    const QString canonical = fileInfo.canonicalFilePath();
    const QString rootPrefix = m_packageRoot + QLatin1Char('/');
    if (canonical != m_packageRoot && !canonical.startsWith(rootPrefix))
    {
        response.status = 403;
        return response;
    }

    QFile file(canonical);
    if (!file.open(QIODevice::ReadOnly))
    {
        return response;
    }

    response.ok = true;
    response.status = 200;
    response.data = file.readAll();
    response.mimeType = mimeTypeForSuffix(canonical);
    return response;
}

} // namespace MiniAppRuntime
