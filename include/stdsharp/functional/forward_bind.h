#pragma once

#include "../functional/invoke.h"
#include "../type_traits/indexed_traits.h"
#include "stdsharp/utility/forward_cast.h"

namespace stdsharp::details
{
    struct forward_bind_traits
    {
    private:
        template<typename T>
        using wrap_t = stdsharp::
            value_wrapper<std::conditional_t<lvalue_ref<T>, T, std::remove_reference_t<T>>>;

        template<typename Binder, typename T>
        using unwrap_t = decltype(std::declval<forward_like_t<Binder, T>>().get());

        struct bind_front
        {
            template<
                typename Self,
                typename... Args,
                std::invocable<Args..., unwrap_t<Self, BindArgs>...> Fn =
                    forward_like_t<Self, func_t>>
            constexpr decltype(auto) operator()(
                this Self&& self,
                const back_tag /*unused*/,
                Args&&... args //
            ) noexcept(nothrow_invocable<Fn, Args..., unwrap_t<Self, BindArgs>...>)
            {
                auto&& binder = self_cast(cpp_forward(self));

                return stdsharp::invoke(
                    cpp_forward(binder).func,
                    cpp_forward(args)...,
                    cpp_forward(binder).args.template get<I>().get()...
                );
            }
        };

        struct back_tag
        {
            template<
                typename Self,
                typename... Args,
                std::invocable<unwrap_t<Self, BindArgs>..., Args...> Fn =
                    forward_like_t<Self, func_t>>
            constexpr decltype(auto) operator()(
                this Self&& self,
                const front_tag /*unused*/,
                Args&&... args //
            ) noexcept(nothrow_invocable<Fn, unwrap_t<Self, BindArgs>..., Args...>)
            {
                auto&& binder = self_cast(cpp_forward(self));

                return stdsharp::invoke(
                    cpp_forward(binder).func,
                    cpp_forward(binder).args.template get<I>().get()...,
                    cpp_forward(args)...
                );
            }
        };

        template<typename Func, typename Idx, typename... BindArgs>
        struct binder;

        template<typename Func, std::size_t... I, typename... BindArgs>
        struct binder<Func, std::index_sequence<I...>, BindArgs...>
        {
        private:
            static constexpr auto self_cast = fwd_cast<binder>;

        public:
            using func_t = std::decay_t<Func>;
            using args_t = stdsharp::indexed_values<wrap_t<BindArgs>...>;

            func_t func;
            args_t args;

        };

    public:

    };
}

namespace stdsharp
{
    using forward_bind_front_fn = details::forward_bind_fn<details::forward_bind_front_binder>;

    inline constexpr forward_bind_front_fn forward_bind_front{};

    using forward_bind_back_fn = details::forward_bind_fn<details::forward_bind_back_binder>;

    inline constexpr forward_bind_back_fn forward_bind_back{};
}
