#pragma once

#include "../namespace_alias.h"

#include <concepts>

namespace stdsharp
{
    template<template<typename...> typename Inner>
    struct adl_proof_traits
    {
        template<typename... T>
        struct types
        {
            using traits = adl_proof_traits;

            using inner_t = Inner<T...>;

            struct proofed_t : inner_t
            {
                using inner_t::inner_t;
            };

        private:
            friend consteval types get_types(const proofed_t&);
        };
    };

    template<template<typename...> typename Inner, typename... T>
    using adl_proof_t = adl_proof_traits<Inner>::template types<T...>::proofed_t;

    template<typename T>
    using adl_proof_inner_t = typename decltype(get_types(std::declval<T>()))::inner_t;
}

namespace stdsharp::details
{
    template<
        typename T,
        template<typename...>
        typename Proofed,
        typename = decltype(get_types(std::declval<T>()))>
    struct adl_proofed_for;

    template<
        typename T,
        template<typename...>
        typename Proofed,
        template<typename...>
        typename Types,
        typename... U>
    struct adl_proofed_for<T, Proofed, Types<U...>>
    {
        static constexpr auto value = std::same_as<T, adl_proof_t<Proofed, U...>>;
    };
}

namespace stdsharp
{
    template<typename T, template<typename...> typename Proofed>
    concept adl_proofed_for = requires { requires details::adl_proofed_for<T, Proofed>::value; };
}
