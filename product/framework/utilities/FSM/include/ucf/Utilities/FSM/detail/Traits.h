#pragma once

/// @file detail/Traits.h
/// @brief Type traits, compile-time utilities, and concepts for the FSM.

#include <concepts>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <variant>

namespace fsm {

// ============================================================================
// Type traits
// ============================================================================

namespace detail {

template <typename T>
struct is_variant : std::false_type {};

template <typename... Ts>
struct is_variant<std::variant<Ts...>> : std::true_type {};

template <typename T>
inline constexpr bool is_variant_v = is_variant<T>::value;

template <typename T, typename... Ts>
inline constexpr bool is_one_of_v = (std::is_same_v<T, Ts> || ...);

template <typename T, typename... Ts>
struct index_of;

template <typename T, typename... Rest>
struct index_of<T, T, Rest...> : std::integral_constant<std::size_t, 0>
{};

template <typename T, typename First, typename... Rest>
struct index_of<T, First, Rest...>
    : std::integral_constant<std::size_t, 1 + index_of<T, Rest...>::value>
{};

template <typename T>
struct index_of<T> : std::integral_constant<std::size_t, 0>
{
    static_assert(sizeof(T) == 0, "Type not found in pack.");
};

// ============================================================================
// Compile-time type name
// ============================================================================

/// Compile-time type name extraction via compiler intrinsics.
/// Works on GCC, Clang, and MSVC. Falls back to "<unknown>" otherwise.
template <typename T>
constexpr std::string_view type_name() noexcept
{
#if defined(__clang__) || defined(__GNUC__)
    // Clang: "... type_name() [T = MyState]"
    // GCC:   "... type_name() [with T = MyState; ...]"  or  "... [with T = MyState]"
    constexpr std::string_view p = __PRETTY_FUNCTION__;
    constexpr std::string_view marker = "T = ";
    constexpr auto pos = p.find(marker);
    static_assert(pos != std::string_view::npos, "Cannot parse __PRETTY_FUNCTION__");
    constexpr auto start = pos + marker.size();
    // GCC may append "; std::string_view = ..." after the type; Clang ends with ']'.
    // Pick whichever delimiter comes first.
    constexpr auto end_semi = p.find(';', start);
    constexpr auto end_bracket = p.rfind(']');
    constexpr auto end = (end_semi < end_bracket) ? end_semi : end_bracket;
    static_assert(end != std::string_view::npos && end > start,
                  "Cannot parse __PRETTY_FUNCTION__");
    return p.substr(start, end - start);
#elif defined(_MSC_VER)
    // MSVC: "... type_name<MyState>(void)"
    constexpr std::string_view p = __FUNCSIG__;
    constexpr std::string_view marker = "type_name<";
    constexpr auto pos = p.find(marker);
    static_assert(pos != std::string_view::npos, "Cannot parse __FUNCSIG__");
    constexpr auto start = pos + marker.size();
    constexpr auto end = p.rfind(">(");
    static_assert(end != std::string_view::npos && end > start,
                  "Cannot parse __FUNCSIG__");
    return p.substr(start, end - start);
#else
    return "<unknown>";
#endif
}

} // namespace detail

// ============================================================================
// Concepts
// ============================================================================

template <typename State, typename Context, typename Event>
concept EventHandler = requires(State& s, Context& ctx, const Event& e) {
    s.onEvent(ctx, e);
};

template <typename State, typename Context>
concept HasOnEnter = requires(State& s, Context& ctx) {
    { s.onEnter(ctx) } -> std::same_as<void>;
};

template <typename State, typename Context>
concept HasOnExit = requires(State& s, Context& ctx) {
    { s.onExit(ctx) } -> std::same_as<void>;
};

template <typename State>
concept HasStateName = requires {
    { State::name() } -> std::convertible_to<std::string_view>;
};

} // namespace fsm
