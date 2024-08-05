#pragma once

#include "type.h"

#include <ranges>
#include <utility>

namespace stdsharp
{
    template<auto... V>
    struct regular_value_sequence
    {
        [[nodiscard]] static constexpr std::size_t size() noexcept { return sizeof...(V); }
    };
}

namespace stdsharp::details
{
    template<template<auto...> typename T, auto... V>
    consteval regular_value_sequence<V...> to_regular_value_sequence(const T<V...>&);

    template<typename T, auto... V>
    consteval regular_value_sequence<V...>
        to_regular_value_sequence(std::integer_sequence<T, V...>);

    template<auto From, auto PlusF, std::size_t... I>
        requires requires { regular_value_sequence<invoke(PlusF, From, I)...>{}; }
    consteval regular_value_sequence<invoke(PlusF, From, I)...>
        make_value_sequence(std::index_sequence<I...>) noexcept;

    template<std::array Array, std::size_t... Index>
        requires nttp_able<typename decltype(Array)::value_type>
    consteval regular_value_sequence<(Array[Index])...>
        array_to_sequence(std::index_sequence<Index...>);

    template<
        constant_value T,
        std::ranges::input_range Range = decltype(T::value),
        nttp_able ValueType = std::ranges::range_value_t<Range>>
        requires requires {
            requires std::ranges::sized_range<Range>;
            requires std::copyable<ValueType>;
        }
    struct rng_to_sequence
    {
        static constexpr auto rng = T::value;
        static constexpr auto size = std::ranges::size(rng);

        static constexpr auto array = []
        {
            if constexpr(requires { array_to_sequence<rng>(std::make_index_sequence<size>{}); })
                return rng;
            else
            {
                std::array<ValueType, size> array{};
                std::ranges::copy(rng, array.begin());
                return array;
            }
        }();

        using type = decltype(array_to_sequence<array>(std::make_index_sequence<array.size()>{}));
    };
}

namespace stdsharp
{
    template<typename Seq>
    using to_regular_value_sequence =
        decltype(details::to_regular_value_sequence(std::declval<Seq>()));

    template<typename T, T Size>
    using make_integer_sequence = to_regular_value_sequence<std::make_integer_sequence<T, Size>>;

    template<std::size_t N>
    using make_index_sequence = make_integer_sequence<std::size_t, N>;

    template<auto From, std::size_t Size, auto PlusF = std::plus{}>
    using make_value_sequence =
        decltype(details::make_value_sequence<From, PlusF>(std::make_index_sequence<Size>{}));

    template<typename... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;

    template<typename Rng>
    using rng_to_sequence = details::rng_to_sequence<Rng>::type;

    template<auto Rng>
    using rng_v_to_sequence = rng_to_sequence<constant<Rng>>;
}

namespace std
{
    template<auto... V>
    struct tuple_size<::stdsharp::regular_value_sequence<V...>>
    {
        static constexpr auto value = ::stdsharp::regular_value_sequence<V...>::size();
    };
}
