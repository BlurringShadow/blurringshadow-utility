#pragma once

#include "../iterator/iterator.h"
#include "../utility/cast_to.h"
#include "../utility/fwd_cast.h"

#include <gsl/gsl_assert>

#include <ranges>

namespace stdsharp
{
    template<typename T>
    using const_iterator_t =
#if __cpp_lib_ranges_as_const >= 202207L
        std::ranges::const_iterator_t<T>;
#else
        decltype(std::ranges::cbegin(std::declval<T&>()));
#endif

    template<typename T>
    using const_sentinel_t =
#if __cpp_lib_ranges_as_const >= 202207L
        std::ranges::const_sentinel_t<T>;
#else
        decltype(std::ranges::cend(std::declval<T&>()));
#endif

    template<typename T>
    using range_const_reference_t =
#if __cpp_lib_ranges_as_const >= 202207L
        std::ranges::range_const_reference_t<T>;
#else
        iter_const_reference_t<std::ranges::iterator_t<T>>;
#endif

#define STDSHARP_MD_RANGE_TRAITS(name, ns)                             \
    template<std::size_t N, typename T>                                \
    struct md_##name                                                   \
    {                                                                  \
        using rng = T;                                                 \
                                                                       \
        using type = ns::name##_t<typename md_##name<N - 1, T>::type>; \
    };                                                                 \
                                                                       \
    template<typename T>                                               \
    struct md_##name<0, T>                                             \
    {                                                                  \
        using rng = T;                                                 \
                                                                       \
        using type = ns::name##_t<T>;                                  \
    };                                                                 \
                                                                       \
    template<std::size_t N, typename T>                                \
    using md_##name##_t = typename md_##name<N, T>::type;

    STDSHARP_MD_RANGE_TRAITS(iterator, std::ranges)
    STDSHARP_MD_RANGE_TRAITS(const_iterator, stdsharp)
    STDSHARP_MD_RANGE_TRAITS(sentinel, std::ranges)
    STDSHARP_MD_RANGE_TRAITS(const_sentinel, stdsharp)
    STDSHARP_MD_RANGE_TRAITS(range_size, std::ranges)
    STDSHARP_MD_RANGE_TRAITS(range_difference, std::ranges)
    STDSHARP_MD_RANGE_TRAITS(range_value, std::ranges)
    STDSHARP_MD_RANGE_TRAITS(range_reference, std::ranges)
    STDSHARP_MD_RANGE_TRAITS(range_const_reference, stdsharp)
    STDSHARP_MD_RANGE_TRAITS(range_rvalue_reference, std::ranges)
    STDSHARP_MD_RANGE_TRAITS(range_common_reference, std::ranges)

#undef STDSHARP_MD_RANGE_TRAITS

    template<typename T>
    using forwarding_view = std::ranges::transform_view<T, forward_like_fn<T>>;

    template<typename T, typename U>
    using cast_view = std::ranges::transform_view<T, cast_to_fn<U>>;

    template<typename T>
    concept constant_iterator = std::input_iterator<T> &&
        std::same_as<iter_const_reference_t<T>, std::iter_reference_t<T>>;

    template<typename T>
    concept constant_range =
#if __cpp_lib_ranges_as_const >= 202207L
        std::ranges::constant_range<T>;
#else
        std::ranges::input_range<T> && constant_iterator<std::ranges::iterator_t<T>>;
#endif

    template<typename Out, typename In>
    concept range_movable = std::ranges::input_range<In> &&
        std::ranges::output_range<Out, std::ranges::range_rvalue_reference_t<In>> &&
        std::indirectly_movable<std::ranges::iterator_t<In>, std::ranges::iterator_t<Out>>;

    template<typename Out, typename In>
    concept range_copyable = std::ranges::input_range<In> &&
        std::ranges::output_range<Out, std::ranges::range_reference_t<Out>> &&
        std::indirectly_copyable<std::ranges::iterator_t<In>, std::ranges::iterator_t<Out>>;

    inline constexpr struct is_iter_in_fn
    {
        template<typename In, std::sentinel_for<In> Sentinel>
            requires std::sentinel_for<In, In>
        constexpr bool operator()(In begin, const Sentinel& end, const In& in) const noexcept
        {
            if(!std::is_constant_evaluated()) return begin <= in && in < end;

            for(; begin != end; ++begin)
                if(begin == in) return true;

            return false;
        }

        template<std::ranges::range R, typename ConstIter = const_iterator_t<R>>
            requires std::invocable<is_iter_in_fn, ConstIter, ConstIter, ConstIter>
        constexpr bool operator()(R&& r, const ConstIter& in) const noexcept
        {
            return (*this)(
                std::ranges::cbegin(cpp_forward(r)),
                std::ranges::cend(cpp_forward(r)),
                in
            );
        }
    } is_iter_in{};

    inline constexpr struct index_fn
    {
        template<
            std::ranges::random_access_range R,
            typename Diff = std::ranges::range_difference_t<R>>
        constexpr decltype(auto) operator()(R&& r, const Diff& i) const
        {
            return std::ranges::begin(cpp_forward(r))[i];
        }

        template<typename Rng, typename FirstArg, typename... Args>
            requires requires {
                requires sizeof...(Args) > 0;
                requires std::invocable<index_fn, Rng, FirstArg>;
                requires std::
                    invocable<index_fn, std::invoke_result_t<index_fn, Rng, FirstArg>, Args...>;
            }
        [[nodiscard]] constexpr decltype(auto) operator()(
            auto&& rng,
            auto&& first_arg,
            auto&&... args //
        ) const
        {
            return (*this)((*this)(cpp_forward(rng), cpp_forward(first_arg)), cpp_forward(args)...);
        }
    } index{};

    inline constexpr struct index_at_fn
    {
        template<typename R, typename Diff = std::ranges::range_difference_t<R>>
            requires std::ranges::sized_range<R> && std::invocable<index_fn, R, const Diff&>
        constexpr decltype(auto) operator()(R&& r, const Diff& i) const
        {
            Expects(i < std::ranges::size(r));
            return index(cpp_forward(r), i);
        }

        template<typename Rng, typename FirstArg, typename... Args>
            requires requires {
                requires sizeof...(Args) > 0;
                requires std::invocable<index_at_fn, Rng, FirstArg>;
                requires std::invocable<
                    index_at_fn,
                    std::invoke_result_t<index_at_fn, Rng, FirstArg>,
                    Args...>;
            }
        [[nodiscard]] constexpr decltype(auto) operator()(
            auto&& rng,
            auto&& first_arg,
            auto&&... args //
        ) const
        {
            return (*this)((*this)(cpp_forward(rng), cpp_forward(first_arg)), cpp_forward(args)...);
        }
    } index_at{};
}

namespace stdsharp::views
{
    template<typename T>
    inline constexpr auto forwarding = std::views::transform(forward_like<T>);

    template<typename U>
    inline constexpr auto cast = std::views::transform(cast_to<U>);
}
