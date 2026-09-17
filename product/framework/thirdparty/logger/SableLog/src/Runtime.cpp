#include <SableLog/Runtime.h>

#include "LoggerBackend.h"
#include "OutputFactory.h"

#include <cstdint>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace sablelog {

class Runtime::Impl final
{
public:
    explicit Impl(RuntimeConfig config);

    [[nodiscard]] std::uint32_t resolveLogger(std::string_view loggerName) const;
    void submit(std::uint32_t backendId, Level level, std::string_view category,
                std::string_view message, std::source_location location) const noexcept;

    void flush() noexcept;
    void shutdown() noexcept;

private:
    enum class State : std::uint8_t
    {
        Running,
        Stopping,
        Stopped,
    };

    std::vector<std::unique_ptr<detail::LoggerBackend>> mBackends;
    std::unordered_map<std::string, std::uint32_t> mBackendIds;

    mutable std::shared_mutex mAdmissionMutex;
    std::mutex mControlMutex;
    State mState{State::Running};
};

Runtime::Impl::Impl(RuntimeConfig config)
{
    if (config.loggers.empty())
    {
        throw std::invalid_argument{"SableLog requires at least one logger"};
    }
    if (config.loggers.size() > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::invalid_argument{"SableLog has too many loggers"};
    }

    std::unordered_set<std::string> names;
    names.reserve(config.loggers.size());

    for (const auto& loggerConfig : config.loggers)
    {
        if (loggerConfig.loggerName.empty())
        {
            throw std::invalid_argument{"SableLog logger name is empty"};
        }
        if (!names.emplace(loggerConfig.loggerName).second)
        {
            throw std::invalid_argument{"SableLog logger name is duplicated"};
        }
    }

    detail::OutputFactory::normalizeAndValidate(config);

    mBackends.reserve(config.loggers.size());
    mBackendIds.reserve(config.loggers.size());

    for (const auto& loggerConfig : config.loggers)
    {
        const auto backendId = static_cast<std::uint32_t>(mBackends.size());
        mBackendIds.emplace(loggerConfig.loggerName, backendId);
        mBackends.emplace_back(std::make_unique<detail::LoggerBackend>(loggerConfig));
    }
}

std::uint32_t Runtime::Impl::resolveLogger(std::string_view loggerName) const
{
    std::shared_lock admissionLock{mAdmissionMutex};

    if (mState != State::Running)
    {
        throw std::logic_error{"SableLog runtime is not running"};
    }
    if (loggerName.empty())
    {
        throw std::invalid_argument{"SableLog logger name is empty"};
    }

    const auto iterator = mBackendIds.find(std::string{loggerName});
    if (iterator == mBackendIds.end())
    {
        throw std::invalid_argument{"SableLog logger is not configured: " + std::string{loggerName}};
    }

    return iterator->second;
}

void Runtime::Impl::submit(std::uint32_t backendId, Level level, std::string_view category,
                           std::string_view message, std::source_location location) const noexcept
{
    try
    {
        std::shared_lock admissionLock{mAdmissionMutex};

        if (mState != State::Running || backendId >= mBackends.size())
        {
            return;
        }

        mBackends[backendId]->enqueue(level, category, message, location);
    }
    catch (...)
    {
        // Logging is best-effort and never propagates exceptions.
    }
}

void Runtime::Impl::flush() noexcept
{
    try
    {
        std::lock_guard controlLock{mControlMutex};
        std::vector<std::future<void>> completions;

        try
        {
            completions.reserve(mBackends.size());
        }
        catch (...)
        {
            return;
        }

        {
            std::unique_lock admissionLock{mAdmissionMutex};

            if (mState != State::Running)
            {
                return;
            }

            for (auto& backend : mBackends)
            {
                try
                {
                    completions.emplace_back(backend->requestFlush());
                }
                catch (...)
                {
                    // Continue requesting flush from the remaining Backends.
                }
            }
        }

        for (auto& completion : completions)
        {
            try
            {
                completion.wait();
            }
            catch (...)
            {
                // Continue waiting for the remaining Backends.
            }
        }
    }
    catch (...)
    {
        // Public logging control operations never propagate exceptions.
    }
}

void Runtime::Impl::shutdown() noexcept
{
    try
    {
        std::lock_guard controlLock{mControlMutex};

        {
            std::unique_lock admissionLock{mAdmissionMutex};

            if (mState == State::Stopped)
            {
                return;
            }

            mState = State::Stopping;
        }

        for (auto& backend : mBackends)
        {
            backend->shutdown();
        }

        {
            std::unique_lock admissionLock{mAdmissionMutex};
            mState = State::Stopped;
        }
    }
    catch (...)
    {
        // Public logging control operations never propagate exceptions.
    }
}

Runtime::Runtime(RuntimeConfig config) : mImpl(std::make_unique<Impl>(std::move(config)))
{
}

Runtime::~Runtime()
{
    shutdown();
}

std::uint32_t Runtime::resolveLogger(std::string_view loggerName) const
{
    if (!mImpl)
    {
        throw std::logic_error{"SableLog runtime is not initialized"};
    }

    return mImpl->resolveLogger(loggerName);
}

void Runtime::submit(std::uint32_t backendId, Level level, std::string_view category,
                     std::string_view message, std::source_location location) const noexcept
{
    if (mImpl)
    {
        mImpl->submit(backendId, level, category, message, location);
    }
}

void Runtime::flush() noexcept
{
    if (mImpl)
    {
        mImpl->flush();
    }
}

void Runtime::shutdown() noexcept
{
    if (mImpl)
    {
        mImpl->shutdown();
    }
}

} // namespace sablelog
