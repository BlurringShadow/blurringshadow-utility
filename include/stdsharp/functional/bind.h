#pragma once

#include "invocables.h"

namespace stdsharp
{
    inline constexpr struct bind_front_fn
    {
        [[nodiscard]] constexpr auto operator()(auto&&... args) const
            noexcept(noexcept(std::bind_front(cpp_forward(args)...))) //
            -> decltype(std::bind_front(cpp_forward(args)...))
        {
            return std::bind_front(cpp_forward(args)...);
        }
    } bind_front{};

#if __cpp_lib_bind_back >= 202202L
    inline constexpr struct bind_back_fn
    {
        [[nodiscard]] constexpr auto operator()(auto&&... args) const
            noexcept(noexcept(std::bind_back(cpp_forward(args)...))) //
            -> decltype(std::bind_back(cpp_forward(args)...))
        {
            return std::bind_back(cpp_forward(args)...);
        }
    } bind_back{};
#endif
}

namespace stdsharp::details
{
    struct forward_bind_traits
    {
    private:
        template<typename T, std::size_t I>
        using forward_indexed_at_t = forward_like_t<T, typename std::decay_t<T>::template type<I>>;

    public:
        template<typename Func>
        struct seq_invoker_fn
        {
            stdsharp::invocables<Func> func;

            template<
                typename Indexed,
                typename... T,
                typename Seq = std::decay_t<Indexed>::index_sequence,
                std::invocable<Seq, Indexed, T...> Self>
            constexpr decltype(auto) operator()(
                this Self&& self,
                Indexed&& indexed,
                T&&... call_args //
            ) noexcept(nothrow_invocable<Self, Seq, Indexed, T...>)
            {
                return cpp_forward(self)(Seq{}, cpp_forward(indexed), cpp_forward(call_args)...);
            }
        };

        template<typename Func>
        struct front_invoker_fn : seq_invoker_fn<Func>
        {
            using seq_invoker_fn<Func>::operator();

            template<std::size_t... I, typename Indexed, typename... T>
                requires std::invocable<Func, forward_indexed_at_t<Indexed, I>..., T...>
            constexpr decltype(auto) operator()(
                this auto&& self,
                const std::index_sequence<I...> /*unused*/,
                Indexed&& indexed,
                T&&... call_args
            ) noexcept(nothrow_invocable<Func, forward_indexed_at_t<Indexed, I>..., T...>)
            {
                return cpp_forward(self).func(
                    cpp_forward(indexed).template get<I>()...,
                    cpp_forward(call_args)...
                );
            }
        };

        template<typename Func>
        struct back_invoker_fn : seq_invoker_fn<Func>
        {
            using seq_invoker_fn<Func>::operator();

            template<std::size_t... I, typename Indexed, typename... T>
                requires std::invocable<Func, T..., forward_indexed_at_t<Indexed, I>...>
            constexpr decltype(auto) operator()(
                this auto&& self,
                const std::index_sequence<I...> /*unused*/,
                Indexed&& indexed,
                T&&... call_args
            ) noexcept(nothrow_invocable<Func, T..., forward_indexed_at_t<Indexed, I>...>)
            {
                return cpp_forward(self).func(
                    cpp_forward(call_args)...,
                    cpp_forward(indexed).template get<I>()...
                );
            }
        };

        template<template<typename> typename BindInvoker>
        struct make_bind
        {
            template<
                typename Func,
                typename... Binds,
                list_initializable_from<Func> Invoker = BindInvoker<std::decay_t<Func>>,
                std::constructible_from<Binds...> Args = stdsharp::indexed_values<
                    std::conditional_t<lvalue_ref<Binds>, Binds, std::remove_cvref_t<Binds>>...>>
                requires std::invocable<bind_front_fn, Invoker, Args>
            constexpr auto operator()(Func&& func, Binds&&... binds) const
                noexcept(nothrow_list_initializable_from<Invoker, Func> && nothrow_invocable<Args, Binds...> && nothrow_invocable<bind_front_fn, Invoker, Args>)
            {
                return bind_front(Invoker{cpp_forward(func)}, Args{cpp_forward(binds)...});
            }
        };
    };
}

namespace stdsharp
{
    using forward_bind_front_fn =
        details::forward_bind_traits::make_bind<details::forward_bind_traits::front_invoker_fn>;

    inline constexpr forward_bind_front_fn forward_bind_front{};

    using forward_bind_back_fn =
        details::forward_bind_traits::make_bind<details::forward_bind_traits::back_invoker_fn>;

    inline constexpr forward_bind_back_fn forward_bind_back{};
}
