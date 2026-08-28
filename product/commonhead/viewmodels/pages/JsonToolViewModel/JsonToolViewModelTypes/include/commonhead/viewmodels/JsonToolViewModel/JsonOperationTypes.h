#pragma once

#include <string>

#include <commonhead/viewmodels/JsonToolViewModel/JsonToolViewModelTypesExport.h>

namespace commonHead::viewModels {

struct JSON_TOOL_VIEW_MODEL_TYPES_API JsonFormatResult final
{
    bool success{false};
    std::string data;
    std::string errorMessage;
};

} // namespace commonHead::viewModels
