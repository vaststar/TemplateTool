#pragma once

#include <QString>
#include <QStringList>

namespace MiniAppRuntime {

// Describes one mini-app instance passed to the runtime at launch. All paths
// are absolute; the runtime only reads this data.
struct MiniAppContext
{
    QString id;              // Unique app id (== package dir name), e.g. "com.acme.crm"
    QString name;            // Display name (window title)
    QString entry;           // Entry file relative to packageDir, e.g. "index.html"
    QString packageDir;      // Absolute, read-only package root (asset source of truth)
    QString storageDir;      // Absolute, persistent per-app storage (may be empty until wired)
    QString cacheDir;        // Absolute, purgeable per-app cache (may be empty until wired)
    QStringList permissions; // Granted capability tokens from the manifest
};

} // namespace MiniAppRuntime
