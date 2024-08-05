#pragma once

#include "../functional/always_return.h"
#include "details/seq_traits.h"
#include "indexed_traits.h"
#include "regular_value_sequence.h"

namespace stdsharp
{
    template<auto...>
    struct value_sequence;

    template<typename T>
    using to_value_sequence = decltype( //
        []<template<auto...> typename Inner, auto... Values>(const Inner<Values...>)
        {
            return value_sequence<Values...>{}; //
        }(std::declval<T>())
    );
}

namespace stdsharp
{
    template<auto... Values>
    struct value_sequence :
        private details::seq_traits<value_sequence<Values...>, sizeof...(Values)>
    {
        static constexpr std::size_t size() noexcept { return sizeof...(Values); }

        template<std::size_t I>
        using value_type = type_at<I, decltype(Values)...>;

        template<std::size_t I>
            requires requires { requires I < size(); }
        [[nodiscard]] static constexpr value_type<I> get() noexcept
        {
            return type_at<I, constant<Values>...>::value;
        }

        template<std::size_t I>
        static constexpr auto const_value = get<I>();

        template<template<auto...> typename T>
        using apply_t = T<Values...>;

    private:
        template<auto... Func>
        struct transform
        {
            using type = regular_value_sequence<stdsharp::invoke(Func, Values)...>;
        };

        template<auto Func>
        struct transform<Func>
        {
            using type = regular_value_sequence<stdsharp::invoke(Func, Values)...>;
        };

        template<typename Func>
            requires(std::invocable<Func&, decltype(Values)> && ...)
        static consteval void invocable_test()
            noexcept((nothrow_invocable<Func&, decltype(Values)> && ...));

        template<typename Func>
            requires(std::predicate<Func&, const decltype(Values)&> && ...)
        static consteval void predicate_test()
            noexcept((nothrow_predicate<Func&, const decltype(Values)&> && ...));

        using m_base = details::seq_traits<value_sequence, size()>;

    public:
        template<auto... Func>
        using transform_t = transform<Func...>::type;

        // TODO: P1306 Expansion statements
        template<typename ResultType = void>
        struct invoke_fn
        {
            template<typename Func>
                requires requires {
                    invocable_test<Func>();
                    requires std::same_as<ResultType, void>;
                }
            constexpr auto operator()(Func&& func) const noexcept(noexcept(invocable_test<Func>()))
            {
                (stdsharp::invoke(func, Values), ...);
            };

            template<typename Func>
                requires requires {
                    invocable_test<Func>();
                    requires std::constructible_from<
                        ResultType,
                        std::invoke_result_t<Func, decltype(Values)>...>;
                }
            constexpr auto operator()(Func&& func) const noexcept( //
                noexcept(invocable_test<Func>()) &&
                nothrow_constructible_from<
                    ResultType,
                    std::invoke_result_t<Func, decltype(Values)>...> //
            )
            {
                return ResultType{stdsharp::invoke(func, Values)...};
            };
        };

        template<typename ResultType = void>
        static constexpr invoke_fn<ResultType> invoke{};

        static constexpr struct for_each_fn
        {
        private:
            static constexpr auto impl(auto&& value, auto& func, auto& condition) noexcept(
                nothrow_invocable<decltype(func), decltype(value)> &&
                nothrow_predicate<decltype(condition), const decltype(value)&> //
            )
            {
                if(!invoke_r<bool>(condition, std::as_const(value))) return false;

                stdsharp::invoke(func, cpp_forward(value));
                return true;
            }

        public:
            template<typename Func, typename Condition = always_true_fn>
                requires requires { invocable_test<Func>(), predicate_test<Condition>(); }
            constexpr void operator()(Func func, Condition condition = {}) const
                noexcept(noexcept(invocable_test<Func>(), predicate_test<Condition>()))
            {
                empty = (impl(Values, func, condition) && ...);
            }
        } for_each{};

        template<auto... Others>
        using append_t = regular_value_sequence<Values..., Others...>;

        template<auto... Others>
        using append_front_t = regular_value_sequence<Others..., Values...>;

        template<std::size_t Index, auto... Other>
        using insert_t =
            m_base::template insert_trait<Index>::template type<regular_value_sequence<Other...>>;

        template<std::size_t Index, auto... Other>
        using replace_t =
            m_base::template replace_trait<Index>::template type<regular_value_sequence<Other...>>;

        template<std::size_t Index>
        using remove_at_t = replace_t<Index>;

        constexpr operator regular_value_sequence<Values...>() const noexcept { return {}; }
    };
}

namespace stdsharp::details
{
    struct value_sequence_algo
    {
        template<typename T, typename Comp>
        struct value_comparer
        {
            using value_type = const T&;

            value_type value;
            Comp& comp;

            template<typename U, typename ConstU = const U&>
                requires std::predicate<Comp&, ConstU, value_type>
            constexpr bool operator()(const U& other) const
                noexcept(nothrow_predicate<Comp&, ConstU, value_type>)
            {
                return invoke(comp, other, value);
            }
        };

        template<typename Seq, typename Fn, typename ForEach = Seq::for_each_fn>
            requires std::invocable<ForEach, Fn>
        static consteval void invocable_test() noexcept(nothrow_invocable<ForEach, Fn>);

        template<typename Seq, typename Predicate, typename ForEach = Seq::for_each_fn>
            requires std::invocable<ForEach, empty_t, Predicate>
        static consteval void predicate_test()
            noexcept(nothrow_invocable<ForEach, empty_t, Predicate>);

        template<typename IfFunc>
        struct if_not_fn
        {
            template<typename Func>
                requires std::invocable<IfFunc, Func&>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(nothrow_invocable<IfFunc, Func&>)
            {
                return IfFunc{}(std::not_fn(std::ref(func)));
            }
        };

        template<typename IfFunc>
        struct from_value_fn
        {
            template<
                typename T,
                typename Comp = std::ranges::equal_to,
                typename Comparer = value_comparer<T, Comp>>
                requires std::invocable<IfFunc, Comparer>
            [[nodiscard]] constexpr auto operator()(const T& v, Comp comp = {}) const
                noexcept(nothrow_invocable<IfFunc, Comparer>)
            {
                return IfFunc{}(Comparer{v, comp});
            }
        };

        template<typename FindFunc, std::size_t Size, bool Equal = true>
        struct result_compare_to_size
        {
            template<typename Func>
                requires std::invocable<FindFunc, Func>
            [[nodiscard]] constexpr auto operator()(Func&& func) const
                noexcept(nothrow_invocable<FindFunc, Func>)
            {
                const auto v = FindFunc{}(cpp_forward(func));
                if constexpr(Equal) return v == Size;
                else return v != Size;
            }
        };
    };
}

namespace stdsharp::value_sequence_algo
{
    template<typename Seq>
    struct find_if_fn
    {
        using algo = details::value_sequence_algo;

        template<typename Func>
            requires requires { algo::predicate_test<Seq, Func>(); }
        [[nodiscard]] constexpr auto operator()(Func func) const
            noexcept(noexcept(algo::predicate_test<Seq, Func>()))
        {
            std::size_t i = 0;
            Seq::for_each(
                [&i](const auto& /*unused*/) noexcept { ++i; },
                std::not_fn(cpp_move(func))
            );
            return i;
        }
    };

    template<typename Seq>
    inline constexpr find_if_fn<Seq> find_if{};

    template<typename Seq>
    struct for_each_n_fn
    {
        using algo = details::value_sequence_algo;

        template<typename Func>
            requires requires { algo::invocable_test<Seq, Func>(); }
        constexpr auto operator()(auto count, Func func) const
            noexcept(noexcept(algo::invocable_test<Seq, Func>()))
        {
            Seq::for_each(
                [&count, &func](const auto& v) noexcept(nothrow_invocable<Func, decltype(v)>)
                {
                    stdsharp::invoke(func, v);
                    --count;
                },
                [&count](const auto& /*unused*/) noexcept { return count > 0; }
            );
        }
    };

    template<typename Seq>
    inline constexpr for_each_n_fn<Seq> for_each_n{};

    template<typename Seq>
    using find_if_not_fn = details::value_sequence_algo::if_not_fn<find_if_fn<Seq>>;

    template<typename Seq>
    inline constexpr find_if_not_fn<Seq> find_if_not{};

    template<typename Seq>
    using find_fn = details::value_sequence_algo::from_value_fn<find_if_fn<Seq>>;

    template<typename Seq>
    inline constexpr find_fn<Seq> find{};

    template<typename Seq>
    struct count_if_fn
    {
        using algo = details::value_sequence_algo;

        template<typename Func>
            requires requires { algo::predicate_test<Seq, Func>(); }
        [[nodiscard]] constexpr auto operator()(Func func) const
            noexcept(noexcept(algo::predicate_test<Seq, Func>()))
        {
            std::size_t i = 0;
            Seq::for_each(
                [&i, &func](const auto& v) noexcept(nothrow_predicate<Func, decltype(v)>)
                {
                    if(invoke_r<bool>(func, v)) ++i;
                }
            );
            return i;
        }
    };

    template<typename Seq>
    inline constexpr count_if_fn<Seq> count_if{};

    template<typename Seq>
    using count_if_not_fn = details::value_sequence_algo::if_not_fn<count_if_fn<Seq>>;

    template<typename Seq>
    inline constexpr count_if_not_fn<Seq> count_if_not{};

    template<typename Seq>
    using count_fn = details::value_sequence_algo::from_value_fn<count_if_fn<Seq>>;

    template<typename Seq>
    inline constexpr count_fn<Seq> count{};

    template<typename Seq>
    using all_of_fn =
        details::value_sequence_algo::result_compare_to_size<find_if_not_fn<Seq>, Seq::size()>;

    template<typename Seq>
    inline constexpr all_of_fn<Seq> all_of{};

    template<typename Seq>
    using none_of_fn =
        details::value_sequence_algo::result_compare_to_size<find_if_fn<Seq>, Seq::size()>;

    template<typename Seq>
    inline constexpr none_of_fn<Seq> none_of{};

    template<typename Seq>
    using any_of_fn =
        details::value_sequence_algo::result_compare_to_size<find_if_fn<Seq>, Seq::size(), false>;

    template<typename Seq>
    inline constexpr any_of_fn<Seq> any_of{};

    template<typename Seq>
    using contains_fn =
        details::value_sequence_algo::result_compare_to_size<find_fn<Seq>, Seq::size(), false>;

    template<typename Seq>
    inline constexpr contains_fn<Seq> contains{};

    template<typename Seq>
    struct adjacent_find_fn
    {
    private:
        static constexpr auto size = Seq::size();

        template<std::size_t I>
        struct by_index
        {
            template<
                typename Comp,
                typename FirstV = const Seq::template value_type<I>&,
                typename SecondV = const Seq::template value_type<I + 1>&>
                requires std::predicate<Comp&, FirstV, SecondV>
            constexpr bool operator()(Comp& comp, std::size_t& i) const
                noexcept(nothrow_predicate<Comp&, FirstV, SecondV>)
            {
                if(invoke(comp, Seq::template const_value<I>, Seq::template const_value<I + 1>))
                    return true;
                ++i;
                return false;
            }
        };

        template<>
        struct by_index<size - 1>
        {
            constexpr bool operator()(auto& /*unused*/, std::size_t& /*unused*/) const noexcept
            {
                return false;
            }
        };

        template<typename Comp, std::size_t... I>
            requires(std::invocable<by_index<I>, Comp&, std::size_t&> && ...)
        static constexpr auto impl(Comp& comp, const std::index_sequence<I...> /*unused*/) //
            noexcept((nothrow_invocable<by_index<I>, Comp&, std::size_t&> && ...))
        {
            std::size_t i{};
            empty = (by_index<I>{}(comp, i) || ...);
            return i;
        }

    public:
        template<
            typename Comp = std::ranges::equal_to,
            typename IdxSeq = std::make_index_sequence<size>>
        [[nodiscard]] constexpr auto operator()(Comp comp = {}) const
            noexcept(noexcept(impl(comp, IdxSeq{})))
            requires requires { impl(comp, IdxSeq{}); }
        {
            return impl(comp, IdxSeq{});
        }
    };

    template<typename Seq>
    inline constexpr adjacent_find_fn<Seq> adjacent_find{};
}

namespace stdsharp::details
{
    template<typename>
    struct reverse_value_sequence;

    template<auto... V>
    struct reverse_value_sequence<value_sequence<V...>>
    {
        using seq = value_sequence<V...>;

        template<std::size_t... I>
        static consteval regular_value_sequence<seq::template get<seq::size() - I - 1>()...>
            impl(std::index_sequence<I...>);

        using type = decltype(impl(std::make_index_sequence<seq::size()>{}));
    };

    template<typename, typename>
    struct unique_value_sequence;

    template<auto... Values, typename Comp>
    struct unique_value_sequence<value_sequence<Values...>, Comp>
    {
        using seq = value_sequence<Values...>;

        static consteval auto size() noexcept { return seq::size(); }

        template<std::size_t I>
        static consteval auto fill_indices(auto& indices, std::size_t& i, Comp& comp)
        {
            const auto found =
                stdsharp::value_sequence_algo::find<seq>(seq::template get<I>(), comp);
            if(found < i) return;
            indices[i++] = I;
        }

        template<auto... I>
        static consteval auto get_indices(const std::index_sequence<I...> /*unused*/)
        {
            std::array<std::size_t, size()> indices{};
            std::size_t i = 0;
            Comp comp{};
            (fill_indices<I>(indices, i, comp), ...);
            return std::pair{indices, i};
        }

        static constexpr auto indices_pair = get_indices(std::make_index_sequence<size()>{});

        static constexpr auto indices = indices_pair.first;

        static constexpr auto indices_size = indices_pair.second;

        template<auto... I>
        static consteval regular_value_sequence<seq::template get<indices[I]>()...>
            apply_indices(std::index_sequence<I...>);

        using type = decltype(apply_indices(std::make_index_sequence<indices_size>{}));
    };
}

namespace stdsharp::value_sequence_algo
{
    template<typename Seq>
    using reverse_t = details::reverse_value_sequence<Seq>::type;

    template<typename Seq, typename Comp = std::ranges::equal_to>
    using unique_t = details::unique_value_sequence<Seq, Comp>::type;
}

namespace std
{
    template<auto... Values>
    struct tuple_size<::stdsharp::value_sequence<Values...>> :
        ::stdsharp::index_constant<::stdsharp::value_sequence<Values...>::size()>
    {
    };

    template<std::size_t I, auto... Values>
    struct tuple_element<I, ::stdsharp::value_sequence<Values...>>
    {
        using type = typename ::stdsharp::value_sequence<Values...>::template value_type<I>;
    };
}
