#pragma once

#include "../regular_value_sequence.h"

#include <utility>

namespace stdsharp
{
    template<auto...>
    struct value_sequence;

    template<typename...>
    struct type_sequence;
}

namespace stdsharp::details
{
    template<typename Seq>
    concept is_type_sequence =
        requires { requires std::same_as<template_rebind<Seq>, type_sequence<>>; };

    template<typename Seq, std::size_t Size>
    struct seq_traits
    {
        template<auto... I, auto... J>
        static consteval auto
            split(std::index_sequence<I...> /*unused*/, std::index_sequence<J...> /*unused*/)
        {
            if constexpr(is_type_sequence<Seq>)
                return []<typename... T>(basic_type_sequence<T...> /*unused*/) consteval
                {
                    return regular_type_sequence<
                        typename Seq::template type<I>...,
                        T...,
                        typename Seq::template type<J>...>{};
                };
            else
                return []<auto... V>(regular_value_sequence<V...> /*unused*/) consteval {
                    return regular_value_sequence<
                        Seq::template get<I>()...,
                        V...,
                        Seq::template get<J>()...>{};
                };
        };

        template<std::size_t Index, std::size_t Offset = 0>
        struct insert_trait
        {
            static constexpr auto impl =
                []<auto... I>(auto tag, std::index_sequence<I...> /*unused*/) consteval
            {
                return split(tag, std::index_sequence<(I + Index + Offset)...>{});
            }(std::make_index_sequence<Index>{}, std::make_index_sequence<Size - Index - Offset>{});

            template<typename U>
            using type = decltype(impl(U{}));
        };

        template<std::size_t Index>
        using replace_trait = insert_trait<Index, 1>;
    };
}
