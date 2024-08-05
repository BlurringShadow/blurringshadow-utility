#pragma once

#include "../concepts/concepts.h"

#include <compare>

namespace stdsharp
{
    template<typename Cat>
    concept ordering_like =
        same_as_any<Cat, std::partial_ordering, std::weak_ordering, std::strong_ordering>;

    constexpr bool is_ud(const std::partial_ordering c) noexcept
    {
        return c == std::partial_ordering::unordered;
    }

    constexpr bool conform_to(const std::partial_ordering l, decltype(l) r) noexcept
    {
        return is_eq(l) || l == r;
    }

    template<typename Fn, typename T, typename U, typename Cat = std::partial_ordering>
    concept ordering_predicate = ordering_like<Cat> && regular_invocable_r<Fn, Cat, T, U>;

    template<typename Fn, typename T, typename U, typename Cat = std::partial_ordering>
    concept nothrow_ordering_predicate = ordering_predicate<Fn, T, U, Cat> &&
        nothrow_invocable_r<Fn, Cat, T, U>;
}
