#pragma once

#include <string>

#include <commonhead/viewmodels/Base64ToolViewModel/Base64ToolViewModelTypesExport.h>

namespace commonHead::viewModels {

struct BASE64_TOOL_VIEW_MODEL_TYPES_API Base64Result final
{
    bool success{false};
    std::string data;
    std::string errorMessage;
};

} // namespace commonHead::viewModels
