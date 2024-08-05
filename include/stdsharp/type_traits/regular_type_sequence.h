#pragma once

#include "../utility/adl_proof.h"

#include <tuple>

namespace stdsharp
{
    template<typename... T>
    struct basic_type_sequence
    {
        [[nodiscard]] static constexpr auto size() noexcept { return sizeof...(T); }
    };

    template<typename... T>
    using regular_type_sequence = adl_proof_t<basic_type_sequence, T...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::basic_type_sequence<T...>>
    {
        static constexpr auto value = ::stdsharp::basic_type_sequence<T...>::size();
    };

    template<::stdsharp::adl_proofed_for<::stdsharp::basic_type_sequence> T>
    struct tuple_size<T> : tuple_size<::stdsharp::adl_proof_inner_t<T>>
    {
    };
}
