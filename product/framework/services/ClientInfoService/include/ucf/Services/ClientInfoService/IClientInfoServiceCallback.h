#pragma once

#include <cstdint>
#include <vector>

namespace ucf::service{

namespace model{
enum class ThemeType;
enum class LanguageType;
}

// Reason for client-info initial load failure. Used by onClientInfoLoadFailed.
enum class ClientInfoLoadError : std::uint8_t {
    DbServiceUnavailable,   // DataWarehouseService could not be resolved at load time.
    DbQueryFailed,          // Reserved: per-row fetch reported an error (not wired today).
};

class IClientInfoServiceCallback
{
public:
    virtual ~ IClientInfoServiceCallback() = default;

    // Fired exactly once after language/theme have been populated from the DB.
    // Late subscribers must check IClientInfoService::isClientInfoReady() because
    // this event will not be re-fired.
    virtual void onClientInfoReady() = 0;

    // Fired exactly once if the initial DB-driven load could not complete.
    // Default empty so existing subscribers do not need to override.
    virtual void onClientInfoLoadFailed(ClientInfoLoadError /*error*/) {}

    // Fired whenever the active language changes (DB load or setApplicationLanguage).
    // Default empty so existing subscribers do not need to override.
    virtual void onClientLanguageChanged(ucf::service::model::LanguageType /*languageType*/) {}

    virtual void onClientThemeChanged(ucf::service::model::ThemeType themeType) = 0;
};
}
