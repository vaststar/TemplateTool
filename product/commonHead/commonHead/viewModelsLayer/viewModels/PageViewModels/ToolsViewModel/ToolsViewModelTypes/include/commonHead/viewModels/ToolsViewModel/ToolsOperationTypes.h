#pragma once

#include <cstdint>
#include <string>

#include <commonHead/viewModels/ToolsViewModel/ToolsViewModelTypesExport.h>

namespace commonHead::viewModels {

/**
 * @brief Base64 操作结果
 */
struct TOOLS_VIEW_MODEL_TYPES_API Base64Result
{
    bool success = false;
    std::string data;
    std::string errorMessage;
};

/**
 * @brief JSON 操作结果
 */
struct TOOLS_VIEW_MODEL_TYPES_API JsonFormatResult
{
    bool success = false;
    std::string data;
    std::string errorMessage;
};

/**
 * @brief 时间戳转换结果
 */
struct TOOLS_VIEW_MODEL_TYPES_API TimestampResult
{
    bool success = false;
    int64_t timestamp = 0;          // Unix 毫秒时间戳
    std::string dateTimeStr;        // 格式化后的日期时间字符串
    std::string timezone;           // 时区
    std::string errorMessage;
};

} // namespace commonHead::viewModels
