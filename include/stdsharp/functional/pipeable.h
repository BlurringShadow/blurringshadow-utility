#pragma once

#include "invocables.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    enum class pipe_mode : std::uint8_t
    {
        none,
        left,
        right
    };

    template<pipe_mode Mode = pipe_mode::left>
    struct pipe_operator;

    template<>
    struct pipe_operator<pipe_mode::left>
    {
        static constexpr auto pipe_mode = pipe_mode::left;

    private:
        template<typename Arg, std::invocable<Arg> Pipe>
            requires decay_derived<Pipe, pipe_operator>
        friend constexpr decltype(auto) operator|(Arg&& arg, Pipe&& pipe)
            noexcept(nothrow_invocable<Pipe, Arg>)
        {
            return invoke(cpp_forward(pipe), cpp_forward(arg));
        }
    };

    template<>
    struct pipe_operator<pipe_mode::right>
    {
        static constexpr auto pipe_mode = pipe_mode::right;

        template<typename Arg, std::invocable<Arg> Pipe>
        constexpr decltype(auto) operator|(this Pipe&& pipe, Arg&& arg)
            noexcept(nothrow_invocable<Pipe, Arg>)
        {
            return invoke(cpp_forward(pipe), cpp_forward(arg));
        }
    };

    template<pipe_mode Mode = pipe_mode::left>
    struct make_pipeable_fn
    {
        template<typename Fn, std::constructible_from<Fn> Invocable = invocables<std::decay_t<Fn>>>
        constexpr auto operator()(Fn&& fn) const noexcept(nothrow_constructible_from<Invocable, Fn>)
        {
            struct STDSHARP_EBO piper : Invocable, pipe_operator<Mode>
            {
            };

            return piper{cpp_forward(fn)};
        }
    };

    template<pipe_mode Mode = pipe_mode::left>
    inline constexpr make_pipeable_fn<Mode> make_pipeable{};
}

#include "../compilation_config_out.h"
