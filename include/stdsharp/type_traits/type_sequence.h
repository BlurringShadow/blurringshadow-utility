#pragma once

#include "value_sequence.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename... T>
    struct type_sequence : private details::seq_traits<type_sequence<T...>, sizeof...(T)>
    {
        using value_seq_t = value_sequence<basic_type_constant<T>{}...>;

        using regular_type_seq = regular_type_sequence<T...>;

        template<auto... Func>
        struct transform
        {
            using type = regular_type_sequence<
                typename decltype(stdsharp::invoke(Func, basic_type_constant<T>{}))::type...>;
        };

        template<auto Func>
        struct transform<Func>
        {
            using type = regular_type_sequence<
                typename decltype(stdsharp::invoke(Func, basic_type_constant<T>{}))::type...>;
        };

        static constexpr std::size_t size() noexcept { return sizeof...(T); }

        template<std::size_t I>
        using type = type_at<I, T...>;

        template<template<typename...> typename U>
        using apply_t = U<T...>;

        template<auto... Func>
        using transform_t = transform<Func...>::type;

        template<typename U = void>
        using invoke_fn = value_seq_t::template invoke_fn<U>;

        template<typename U = void>
        static constexpr invoke_fn<U> invoke{};

        using for_each_fn = value_seq_t::for_each_fn;

        static constexpr for_each_fn for_each{};

        template<typename... Others>
        using append_t = regular_type_sequence<T..., Others...>;

        template<typename... Others>
        using append_front_t = regular_type_sequence<Others..., T...>;

    private:
        using m_base = details::seq_traits<type_sequence, size()>;

    public:
        template<std::size_t Index, typename... Other>
        using insert_t =
            m_base::template insert_trait<Index>::template type<basic_type_sequence<Other...>>;

        template<std::size_t Index, typename... Other>
        using replace_t =
            m_base::template replace_trait<Index>::template type<basic_type_sequence<Other...>>;

        template<std::size_t Index>
        using remove_at_t = replace_t<Index>;
    };

    template<typename T>
    using to_type_sequence = decltype( //
        []<template<typename...> typename Inner, typename... U>(const Inner<U...>&)
        {
            return type_sequence<U...>{}; //
        }(std::declval<T>())
    );
}

namespace stdsharp::details
{
    template<template<auto...> typename Seq, auto... V>
    consteval regular_type_sequence<typename decltype(V)::type...>
        value_seq_convert_to_type_seq(Seq<V...>);

    template<typename Seq>
    using value_seq_convert_to_type_seq_t = decltype(value_seq_convert_to_type_seq(Seq{}));
}

namespace stdsharp::type_sequence_algo
{
    template<typename Seq>
    using find_if_fn = value_sequence_algo::find_if_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr find_if_fn<Seq> find_if{};

    template<typename Seq>
    using for_each_n_fn = value_sequence_algo::for_each_n_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr for_each_n_fn<Seq> for_each_n{};

    template<typename Seq>
    using find_if_not_fn = value_sequence_algo::find_if_not_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr find_if_not_fn<Seq> find_if_not{};

    template<typename Seq>
    using find_fn = value_sequence_algo::find_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr find_fn<Seq> find{};

    template<typename Seq>
    using count_if_fn = value_sequence_algo::count_if_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr count_if_fn<Seq> count_if{};

    template<typename Seq>
    using count_if_not_fn = value_sequence_algo::count_if_not_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr count_if_not_fn<Seq> count_if_not{};

    template<typename Seq>
    using count_fn = value_sequence_algo::count_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr count_fn<Seq> count{};

    template<typename Seq>
    using all_of_fn = value_sequence_algo::all_of_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr all_of_fn<Seq> all_of{};

    template<typename Seq>
    using none_of_fn = value_sequence_algo::none_of_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr none_of_fn<Seq> none_of{};

    template<typename Seq>
    using any_of_fn = value_sequence_algo::any_of_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr any_of_fn<Seq> any_of{};

    template<typename Seq>
    using contains_fn = value_sequence_algo::contains_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr contains_fn<Seq> contains{};

    template<typename Seq>
    using adjacent_find_fn = value_sequence_algo::adjacent_find_fn<typename Seq::value_seq_t>;

    template<typename Seq>
    inline constexpr adjacent_find_fn<Seq> adjacent_find{};

    template<typename Seq>
    using reverse_t = details::
        value_seq_convert_to_type_seq_t<value_sequence_algo::reverse_t<typename Seq::value_seq_t>>;

    template<typename Seq, typename Comp = std::ranges::equal_to>
    using unique_t = details::value_seq_convert_to_type_seq_t< //
        value_sequence_algo::unique_t<typename Seq::value_seq_t, Comp>>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::type_sequence<T...>> :
        stdsharp::index_constant<::stdsharp::type_sequence<T...>::size()>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::type_sequence<T...>>
    {
        using type = typename ::stdsharp::type_sequence<T...>::template type<I>;
    };
}

#include "../compilation_config_out.h"
