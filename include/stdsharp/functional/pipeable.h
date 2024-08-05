#pragma once

#include "invocables.h"

#include <range/v3/functional/pipeable.hpp>

// port from range-v3 pipeable.hpp
// add noexcept feature
namespace stdsharp
{
    enum class pipe_mode : std::uint8_t
    {
        none,
        left,
        right
    };

    template<pipe_mode Mode = pipe_mode::left>
    struct pipeable_base;

    template<>
    struct pipeable_base<pipe_mode::left>
    {
        static constexpr auto pipe_mode = pipe_mode::left;

    private:
        template<typename Arg, std::invocable<Arg> Pipe>
            requires decay_derived<Pipe, pipeable_base>
        friend constexpr decltype(auto) operator|(Arg&& arg, Pipe&& pipe)
            noexcept(nothrow_invocable<Pipe, Arg>)
        {
            return invoke(cpp_forward(pipe), cpp_forward(arg));
        }
    };

    template<>
    struct pipeable_base<pipe_mode::right>
    {
        static constexpr auto pipe_mode = pipe_mode::right;

        template<typename Arg, std::invocable<Arg> Pipe>
        constexpr decltype(auto) operator|(this Pipe&& pipe, Arg&& arg)
            noexcept(nothrow_invocable<Pipe, Arg>)
        {
            return invoke(cpp_forward(pipe), cpp_forward(arg));
        }
    };
}

namespace stdsharp
{
    template<pipe_mode Mode = pipe_mode::left>
    struct make_pipeable_fn
    {
        template<
            typename Fn,
            std::constructible_from<Fn> DecayFn = std::decay_t<Fn>,
            typename Invocable = invocables<DecayFn>>
        constexpr auto operator()(Fn&& fn) const noexcept(nothrow_constructible_from<DecayFn, Fn>)
        {
            struct piper : Invocable, pipeable_base<Mode>
            {
            };

            return piper{cpp_forward(fn)};
        }
    };

    template<pipe_mode Mode = pipe_mode::left>
    inline constexpr make_pipeable_fn<Mode> make_pipeable{};

    template<typename T, pipe_mode Mode = pipe_mode::left>
    concept pipeable = std::derived_from<T, pipeable_base<Mode>> ||
        (Mode == pipe_mode::left && ranges::is_pipeable_v<T>);

    template<typename Pipe>
    inline constexpr auto get_pipe_mode = pipeable<Pipe, pipe_mode::left> ? //
        pipe_mode::left :
        pipeable<Pipe, pipe_mode::right> ? pipe_mode::right : pipe_mode::none;
}
