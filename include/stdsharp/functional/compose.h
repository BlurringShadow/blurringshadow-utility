#pragma once

#include "../functional/invocables.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<std::size_t I = 0, typename Fn>
    static constexpr decltype(auto) composed_invoke(Fn&& fn, auto&&... arg) noexcept( //
        noexcept( //
            composed_invoke<I + 1>(
                cpp_forward(fn),
                cpp_forward(fn).template get<I>()(cpp_forward(arg)...)
            )
        )
    )
        requires requires {
            composed_invoke<I + 1>(
                cpp_forward(fn),
                cpp_forward(fn).template get<I>()(cpp_forward(arg)...)
            );
        }
    {
        return composed_invoke<I + 1>(
            cpp_forward(fn),
            cpp_forward(fn).template get<I>()(cpp_forward(arg)...)
        );
    }

    template<std::size_t I, typename... T>
        requires(I == std::tuple_size_v<invocables<T...>>)
    static constexpr decltype(auto) composed_invoke(
        const invocables<T...>& /*unused*/,
        auto&& arg //
    ) noexcept
    {
        return cpp_forward(arg);
    }
}

namespace stdsharp
{
    template<typename... T>
    class composed : public invocables<T...>
    {
        using invocables = invocables<T...>;

    public:
        using invocables::invocables;

        composed() = default;

        constexpr decltype(auto) operator()(this auto&& self, auto&&... args)
            noexcept(noexcept(details::composed_invoke(cpp_forward(self), cpp_forward(args)...)))
            requires requires { details::composed_invoke(cpp_forward(self), cpp_forward(args)...); }
        {
            return details::composed_invoke(cpp_forward(self), cpp_forward(args)...);
        }
    };

    template<typename... T>
    composed(T&&...) -> composed<std::decay_t<T>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::composed<T...>> : ::std::tuple_size<::stdsharp::invocables<T...>>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::composed<T...>> :
        ::std::tuple_element<I, ::stdsharp::invocables<T...>>
    {
    };
}

#include "../compilation_config_out.h"
