#pragma once

#include "../tuple/get.h"
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
        template<typename Func>
        struct front_invoker_fn
        {
            stdsharp::invocables<Func> func;

            template<std::size_t... I, typename Indexed, typename... T>
                requires std::invocable<Func, get_element_t<I, Indexed>..., T...>
            constexpr decltype(auto) operator()(
                this auto&& self,
                const std::index_sequence<I...> /*unused*/,
                Indexed&& indexed,
                T&&... call_args
            ) noexcept(nothrow_invocable<Func, get_element_t<I, Indexed>..., T...>)
            {
                return cpp_forward(self).func(
                    cpo::get_element<I>(cpp_forward(indexed))...,
                    cpp_forward(call_args)...
                );
            }
        };

        template<typename Func>
        struct back_invoker_fn
        {
            stdsharp::invocables<Func> func;

            template<std::size_t... I, typename Indexed, typename... T>
                requires std::invocable<Func, T..., get_element_t<I, Indexed>...>
            constexpr decltype(auto) operator()(
                this auto&& self,
                const std::index_sequence<I...> /*unused*/,
                Indexed&& indexed,
                T&&... call_args
            ) noexcept(nothrow_invocable<Func, T..., get_element_t<I, Indexed>...>)
            {
                return cpp_forward(self).func(
                    cpp_forward(call_args)...,
                    cpo::get_element<I>(cpp_forward(indexed))...
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
                    std::conditional_t<lvalue_ref<Binds>, Binds, std::decay_t<Binds>>...>,
                typename Seq = Args::index_sequence>
                requires std::invocable<bind_front_fn, Invoker, Seq, Args>
            constexpr auto operator()(Func&& func, Binds&&... binds) const noexcept( //
                nothrow_list_initializable_from<Invoker, Func> &&
                nothrow_constructible_from<Args, Binds...> &&
                nothrow_invocable<bind_front_fn, Invoker, Seq, Args> //
            )
            {
                return bind_front(Invoker{cpp_forward(func)}, Seq{}, Args{cpp_forward(binds)...});
            }
        };

    public:
        using forward_bind_front_fn = make_bind<front_invoker_fn>;

        using forward_bind_back_fn = make_bind<back_invoker_fn>;
    };
}

namespace stdsharp
{
    using forward_bind_front_fn = details::forward_bind_traits::forward_bind_front_fn;

    inline constexpr forward_bind_front_fn forward_bind_front{};

    using forward_bind_back_fn = details::forward_bind_traits::forward_bind_back_fn;

    inline constexpr forward_bind_back_fn forward_bind_back{};
}
