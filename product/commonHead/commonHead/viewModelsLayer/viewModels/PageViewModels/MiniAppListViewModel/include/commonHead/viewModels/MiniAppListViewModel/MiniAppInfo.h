#pragma once

#include <string>
#include <vector>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::viewModels::model {

/**
 * @brief Metadata describing a single installed mini app.
 *
 * Built from the service-layer manifest (see toMiniAppInfo). Path fields the
 * service resolves lazily (storageDir/cacheDir) stay empty until the app is
 * launched.
 */
struct COMMONHEAD_EXPORT MiniAppInfo
{
    std::string id;             // Globally unique id, e.g. "com.customer.crm"
    std::string name;           // Display name shown in the list
    std::string description;    // Short description
    std::string entry;          // Entry point (e.g. relative index.html); unused in shell
    std::string iconPath;       // Absolute path to the icon file; empty => use placeholder
    std::string packageDir;     // Absolute, read-only package root (empty if unknown)
    std::string storageDir;     // Absolute, persistent per-app storage (resolved lazily)
    std::string cacheDir;       // Absolute, purgeable per-app cache (resolved lazily)
    std::vector<std::string> permissions; // Capability tokens granted to the app
};

}
