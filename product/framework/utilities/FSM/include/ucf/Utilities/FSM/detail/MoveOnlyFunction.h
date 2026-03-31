#pragma once

/// @file detail/MoveOnlyFunction.h
/// @brief Lightweight move-only callable wrapper with small-buffer optimization.
///
/// Replaces std::unique_ptr<IAction> to avoid per-event heap allocations.
/// Callables that fit within BufSize bytes are stored inline (SBO);
/// larger callables fall back to heap allocation transparently.

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace fsm::detail {

template <typename Signature, std::size_t BufSize = 64>
class MoveOnlyFunction;

template <typename R, typename... Args, std::size_t BufSize>
class MoveOnlyFunction<R(Args...), BufSize>
{
    static constexpr std::size_t kAlign = alignof(std::max_align_t);

    using InvokeFn  = R (*)(void*, Args...);
    using DestroyFn = void (*)(void*) noexcept;
    using MoveFn    = void (*)(void* dst, void* src) noexcept;

    template <typename F>
    static constexpr bool fits_sbo =
        sizeof(F) <= BufSize &&
        alignof(F) <= kAlign &&
        std::is_nothrow_move_constructible_v<F>;

    alignas(kAlign) unsigned char m_buf[BufSize]{};
    InvokeFn  m_invoke  = nullptr;
    DestroyFn m_destroy = nullptr;
    MoveFn    m_move    = nullptr;

    void reset() noexcept
    {
        if (m_destroy)
            m_destroy(m_buf);
        m_invoke  = nullptr;
        m_destroy = nullptr;
        m_move    = nullptr;
    }

public:
    MoveOnlyFunction() = default;

    MoveOnlyFunction(std::nullptr_t) noexcept {}

    template <typename F>
        requires(!std::is_same_v<std::decay_t<F>, MoveOnlyFunction>
                 && std::is_invocable_r_v<R, std::decay_t<F>&, Args...>)
    MoveOnlyFunction(F&& f) // NOLINT(google-explicit-constructor)
    {
        using Fn = std::decay_t<F>;

        if constexpr (fits_sbo<Fn>)
        {
            ::new (m_buf) Fn(std::forward<F>(f));

            m_invoke = [](void* buf, Args... args) -> R {
                return (*std::launder(reinterpret_cast<Fn*>(buf)))(
                    std::forward<Args>(args)...);
            };
            m_destroy = [](void* buf) noexcept {
                std::launder(reinterpret_cast<Fn*>(buf))->~Fn();
            };
            m_move = [](void* dst, void* src) noexcept {
                ::new (dst) Fn(std::move(*std::launder(reinterpret_cast<Fn*>(src))));
                std::launder(reinterpret_cast<Fn*>(src))->~Fn();
            };
        }
        else
        {
            auto* heap = new Fn(std::forward<F>(f));
            *reinterpret_cast<Fn**>(m_buf) = heap;

            m_invoke = [](void* buf, Args... args) -> R {
                return (**reinterpret_cast<Fn**>(buf))(std::forward<Args>(args)...);
            };
            m_destroy = [](void* buf) noexcept {
                delete *reinterpret_cast<Fn**>(buf);
            };
            m_move = [](void* dst, void* src) noexcept {
                *reinterpret_cast<Fn**>(dst) = *reinterpret_cast<Fn**>(src);
                *reinterpret_cast<Fn**>(src) = nullptr;
            };
        }
    }

    ~MoveOnlyFunction() { reset(); }

    MoveOnlyFunction(const MoveOnlyFunction&) = delete;
    MoveOnlyFunction& operator=(const MoveOnlyFunction&) = delete;

    MoveOnlyFunction(MoveOnlyFunction&& other) noexcept
        : m_invoke(other.m_invoke)
        , m_destroy(other.m_destroy)
        , m_move(other.m_move)
    {
        if (m_move)
            m_move(m_buf, other.m_buf);
        other.m_invoke  = nullptr;
        other.m_destroy = nullptr;
        other.m_move    = nullptr;
    }

    MoveOnlyFunction& operator=(MoveOnlyFunction&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            m_invoke  = other.m_invoke;
            m_destroy = other.m_destroy;
            m_move    = other.m_move;
            if (m_move)
                m_move(m_buf, other.m_buf);
            other.m_invoke  = nullptr;
            other.m_destroy = nullptr;
            other.m_move    = nullptr;
        }
        return *this;
    }

    explicit operator bool() const noexcept { return m_invoke != nullptr; }

    R operator()(Args... args)
    {
        return m_invoke(m_buf, std::forward<Args>(args)...);
    }
};

} // namespace fsm::detail
