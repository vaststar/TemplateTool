#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace MiniAppRuntime {

// Resolves app://<id>/<path> to files inside one mini-app's read-only package
// dir. Access is confined to the package root; escaping paths are rejected.
class AppSchemeHandler
{
public:
    struct Response
    {
        bool ok = false;   // false => not found / rejected
        int status = 404;  // HTTP-like status for the backend to report
        QByteArray data;   // file contents when ok
        QString mimeType;
    };

    explicit AppSchemeHandler(const QString& packageDir);

    [[nodiscard]] Response resolve(const QUrl& url) const;

private:
    QString m_packageRoot; // canonical absolute path, or empty if invalid
};

} // namespace MiniAppRuntime
