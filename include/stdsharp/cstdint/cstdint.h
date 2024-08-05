#pragma once

#include "../namespace_alias.h"

#include <climits>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if __has_include(<stdfloat>)

    #include <stdfloat>

namespace stdsharp
{
    #ifdef __STDCPP_FLOAT16_T__
    using f16 = std::float16_t;
    #endif

    #ifdef __STDCPP_FLOAT32_T__
    using f32 = std::float32_t;
    #endif

    #ifdef __STDCPP_FLOAT64_T__
    using f64 = std::float64_t;
    #endif

    #ifdef __STDCPP_FLOAT128_T__
    using f128 = std::float128_t;
    #endif

    #ifdef __STDCPP_BFLOAT16_T__
    using bf16 = std::bfloat16_t;
    #endif
}

#endif

namespace stdsharp
{
    inline constexpr std::size_t char_bit = CHAR_BIT;

    using ushort = unsigned short;
    using ulong = unsigned long;
    using ull = unsigned long long;

    using byte = std::underlying_type_t<std::byte>;

#ifdef INT8_MAX
    using i8 = std::int8_t;
#endif

#ifdef UINT8_MAX
    using u8 = std::uint8_t;
#endif

#ifdef INT16_MAX
    using i16 = std::int16_t;
#endif

#ifdef UINT16_MAX
    using u16 = std::uint16_t;
#endif

#ifdef INT32_MAX
    using i32 = std::int32_t;
#endif

#ifdef UINT32_MAX
    using u32 = std::uint32_t;
#endif

#ifdef INT64_MAX
    using i64 = std::int64_t;
#endif

#ifdef UINT64_MAX
    using u64 = std::uint64_t;
#endif

    using ssize_t = std::make_signed_t<std::size_t>;

    inline constexpr struct
    {
        template<typename T>
        [[nodiscard]] constexpr auto operator()(const T t) noexcept
        {
            return static_cast<std::make_unsigned_t<T>>(t);
        }
    } make_unsigned{};

    inline constexpr struct
    {
        template<typename T>
        [[nodiscard]] constexpr auto operator()(const T t) noexcept
        {
            return static_cast<std::make_signed_t<T>>(t);
        }
    } make_signed{};

    inline namespace literals
    {
#define STDSHARP_INT_LITERALS(literal)                                                         \
    [[nodiscard]] constexpr auto operator""_##literal(const unsigned long long value) noexcept \
    {                                                                                          \
        return static_cast<literal>(value);                                                    \
    }

#define STDSHARP_SIGNS_INT_LITERALS(num) STDSHARP_INT_LITERALS(i##num) STDSHARP_INT_LITERALS(u##num)

        STDSHARP_SIGNS_INT_LITERALS(8)
        STDSHARP_SIGNS_INT_LITERALS(16)
        STDSHARP_SIGNS_INT_LITERALS(32)
        STDSHARP_SIGNS_INT_LITERALS(64)

#undef STDSHARP_SIGNS_INT_LITERALS

        STDSHARP_INT_LITERALS(byte)

#undef STDSHARP_INT_LITERALS
    }
}
