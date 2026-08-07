#pragma once

#include <string>
#include <vector>

#include <ucf/Services/MiniAppService/MiniAppServiceTypesExport.h>

namespace ucf::service::model {

/// Parsed representation of a mini-app's manifest.json.
/// A mini-app package lives under <packages>/<id>/ and must contain a
/// manifest.json describing it. `id` MUST equal the package directory name.
struct MINI_APP_SERVICE_TYPES_API MiniAppManifest {
    std::string id;                        ///< Unique app id, equals package dir name (required)
    std::string name;                      ///< Human-readable display name (required)
    std::string version;                   ///< Semantic version string, e.g. "1.0.0" (required)
    std::string description;               ///< Optional short description
    std::string entry;                     ///< Entry file relative to package dir, e.g. "index.html"
    std::string icon;                      ///< Optional icon file relative to package dir
    std::vector<std::string> permissions;  ///< Reserved for future use, empty for now
};

} // namespace ucf::service::model
