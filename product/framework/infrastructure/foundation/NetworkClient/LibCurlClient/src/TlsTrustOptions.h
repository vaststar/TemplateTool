#pragma once

#include <curl/curl.h>

#if defined(__linux__) && !defined(__ANDROID__)
#include <filesystem>
#endif

namespace ucf::infrastructure::network::libcurl {

inline CURLcode configureTlsTrust([[maybe_unused]] CURL* handle)
{
#if defined(_WIN32)
    // Vendored curl uses OpenSSL. Import the Windows certificate stores for TLS.
    constexpr auto nativeCa = static_cast<long>(CURLSSLOPT_NATIVE_CA);
    if (const auto result = curl_easy_setopt(handle, CURLOPT_SSL_OPTIONS, nativeCa);
        result != CURLE_OK)
    {
        return result;
    }
    return curl_easy_setopt(handle, CURLOPT_PROXY_SSL_OPTIONS, nativeCa);
#elif defined(__linux__) && !defined(__ANDROID__)
    // The bundled libcurl defaults to a Debian-style path. On Fedora/RHEL,
    // use the system CA package's bundle if the default file is absent.
    char* defaultBundle = nullptr;
    if (const auto result = curl_easy_getinfo(handle, CURLINFO_CAINFO, &defaultBundle);
        result != CURLE_OK)
    {
        return result;
    }

    std::error_code error;
    if (defaultBundle && std::filesystem::is_regular_file(defaultBundle, error))
    {
        return CURLE_OK;
    }

    constexpr auto fedoraBundle = "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem";
    if (!std::filesystem::is_regular_file(fedoraBundle, error))
    {
        return CURLE_OK;
    }
    if (const auto result = curl_easy_setopt(handle, CURLOPT_CAINFO, fedoraBundle);
        result != CURLE_OK)
    {
        return result;
    }
    return curl_easy_setopt(handle, CURLOPT_PROXY_CAINFO, fedoraBundle);
#else
    return CURLE_OK;
#endif
}

} // namespace ucf::infrastructure::network::libcurl
