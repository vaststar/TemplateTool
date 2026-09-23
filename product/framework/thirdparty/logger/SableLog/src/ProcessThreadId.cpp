#if defined(__linux__) && !defined(__ANDROID__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include "ProcessThreadId.h"

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#elif defined(__APPLE__)

#include <pthread.h>
#include <unistd.h>

#elif defined(__ANDROID__)

#include <unistd.h>

#elif defined(__linux__)

#include <sys/syscall.h>
#include <unistd.h>

#else

#error "SableLog process/thread IDs are not implemented for this platform"

#endif

namespace sablelog::detail {

ProcessThreadId currentProcessThreadId() noexcept
{
#if defined(_WIN32)

    const auto processId = static_cast<std::uint64_t>(::GetCurrentProcessId());
    const auto threadId = static_cast<std::uint64_t>(::GetCurrentThreadId());

#elif defined(__APPLE__)

    const auto processId = static_cast<std::uint64_t>(::getpid());

    std::uint64_t threadId{};
    if (::pthread_threadid_np(nullptr, &threadId) != 0)
    {
        threadId = 0U;
    }

#elif defined(__ANDROID__)

    const auto processId = static_cast<std::uint64_t>(::getpid());
    const auto threadId = static_cast<std::uint64_t>(::gettid());

#elif defined(__linux__)

    const auto processId = static_cast<std::uint64_t>(::getpid());
    const auto nativeThreadId = ::syscall(SYS_gettid);
    const auto threadId = nativeThreadId > 0 ? static_cast<std::uint64_t>(nativeThreadId) : std::uint64_t{0};

#endif

    return {processId, threadId};
}

} // namespace sablelog::detail
