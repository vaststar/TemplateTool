#pragma once

#include <functional>
#include <memory>
#include <string>
#include <filesystem>

namespace ucf::service {

/// Abstract interface for platform crash handlers
class IPlatformCrashHandler
{
public:
    /// Crash callback
    using CrashCallback = std::function<void(int signalCode, const char* signalName)>;
    
    /// Configuration struct
    struct Config
    {
        CrashCallback callback;              ///< Callback function when crash occurs
        std::filesystem::path crashDir;      ///< Crash file directory (for minidump, etc.)
    };
    
    virtual ~IPlatformCrashHandler() = default;
    
    /// Factory method: create and install platform crash handler
    /// @param config Configuration parameters
    /// @return Installed handler, auto-uninstalls on destruction
    static std::unique_ptr<IPlatformCrashHandler> create(Config config);
    
    /// Capture stack trace
    /// @param skipFrames Number of stack frames to skip (skip signal handling related frames)
    virtual std::string captureStackTrace(int skipFrames = 0) const = 0;
    
    /// Check if installed
    [[nodiscard]] bool isInstalled() const { return mInstalled; }
    
protected:
    explicit IPlatformCrashHandler(Config config)
        : mCallback(std::move(config.callback))
        , mCrashDir(std::move(config.crashDir)) {}
    
    // Disable copy
    IPlatformCrashHandler(const IPlatformCrashHandler&) = delete;
    IPlatformCrashHandler& operator=(const IPlatformCrashHandler&) = delete;
    IPlatformCrashHandler(IPlatformCrashHandler&&) = delete;
    IPlatformCrashHandler& operator=(IPlatformCrashHandler&&) = delete;
    
    /// Install handler (called in constructor)
    void install();
    
    /// Uninstall handler (called in destructor)
    void uninstall();
    
    /// Subclass implementation of actual install logic
    virtual void doInstall() = 0;
    
    /// Subclass implementation of actual uninstall logic
    virtual void doUninstall() = 0;
    
protected:
    CrashCallback mCallback;
    std::filesystem::path mCrashDir;
    bool mInstalled { false };
};

} // namespace ucf::service
