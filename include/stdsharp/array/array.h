#pragma once

#include "../macros.h"
#include "../namespace_alias.h"

#include <algorithm>
#include <array>
#include <ranges>

namespace stdsharp
{
    template<std::size_t N>
    struct range_to_array_fn
    {
        template<typename Rng, typename Proj = std::identity>
        constexpr auto operator()(Rng&& rng, Proj proj = {}) const
        {
            using value_type = std::projected<std::ranges::iterator_t<Rng>, Proj>::value_type;

            std::array<value_type, N> arr{};

            std::ranges::copy(
                cpp_forward(rng) | std::views::transform(proj) | std::views::take(N),
                arr.begin()
            );

            return arr;
        }
    };

    template<std::size_t N>
    inline constexpr range_to_array_fn<N> range_to_array{};
}
