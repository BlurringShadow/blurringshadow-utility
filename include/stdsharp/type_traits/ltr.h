#pragma once

#include "type.h"

#include <algorithm>
#include <array>
#include <string_view>

namespace stdsharp::inline literals
{
    template<typename CharT, std::size_t Size>
    struct ltr : std::array<CharT, Size>
    {
    private:
        using array_t = const CharT (&)[Size]; // NOLINT(*-avoid-c-arrays)

    public:
        using base = std::array<CharT, Size>;
        using base::base;

        constexpr ltr(array_t arr) noexcept { std::ranges::copy(arr, base::begin()); }

        constexpr ltr& operator=(array_t arr) noexcept
        {
            std::ranges::copy(arr, base::begin());
            return *this;
        }

        [[nodiscard]] constexpr operator std::string_view() const noexcept
        {
            return {base::data(), Size - 1};
        }

        [[nodiscard]] constexpr auto to_string_view() const noexcept
        {
            return static_cast<std::string_view>(*this);
        }
    };

    template<typename CharT, std::size_t Size>
    ltr(const CharT (&)[Size]) -> ltr<CharT, Size>; // NOLINT(*-avoid-c-arrays)

    template<ltr Ltr>
    [[nodiscard]] constexpr auto operator""_ltr() noexcept
    {
        return Ltr;
    }

    template<ltr Ltr>
    using ltr_constant = constant<Ltr>;
}
