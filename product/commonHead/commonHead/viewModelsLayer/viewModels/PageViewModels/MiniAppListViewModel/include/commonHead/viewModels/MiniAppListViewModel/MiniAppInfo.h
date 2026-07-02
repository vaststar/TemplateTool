#pragma once

#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::viewModels::model {

/**
 * @brief Metadata describing a single installed mini app.
 *
 * Parsed from a mini app's manifest. For the current shell stage the fields are
 * populated with placeholder data; later they will be filled from manifest.json.
 */
struct COMMONHEAD_EXPORT MiniAppInfo
{
    std::string id;             // Globally unique id, e.g. "com.customer.crm"
    std::string name;           // Display name shown in the list
    std::string description;    // Short description
    std::string entry;          // Entry point (e.g. relative index.html); unused in shell
};

}
