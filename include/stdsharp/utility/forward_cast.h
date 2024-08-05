#pragma once

#include "../type_traits/type.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename From, typename To>
    using forward_cast_t = apply_qualifiers<
        To,
        const_<std::remove_reference_t<From>>,
        volatile_<std::remove_reference_t<From>>,
        ref_qualifier_v<From&&>>;

    template<typename From, typename To, typename... Rest>
    struct forward_cast_fn
    {
    private:
        using current_fn = forward_cast_fn<From, To>;
        using current_cast_t = typename current_fn::cast_t;
        using next_fn = forward_cast_fn<current_cast_t, Rest...>;

    public:
        using from_t = std::remove_reference_t<From>;

        [[nodiscard]] constexpr decltype(auto) operator()(from_t&& from) const noexcept
            requires std::invocable<current_fn, from_t> && std::invocable<next_fn, current_cast_t>
        {
            return next_fn{}(current_fn{}(cpp_move(from)));
        }

        [[nodiscard]] constexpr decltype(auto) operator()(from_t& from) const noexcept
            requires std::invocable<current_fn, from_t&> && std::invocable<next_fn, current_cast_t>
        {
            return next_fn{}(current_fn{}(from));
        }
    };

    template<typename From, typename To>
        requires not_decay_derived<From, To> && not_decay_derived<To, From> &&
        not_same_as<std::remove_cvref_t<From>, std::remove_cvref_t<To>>
    struct forward_cast_fn<From, To>
    {
    };

    template<typename From, typename To>
    struct forward_cast_fn<From, To>
    {
        using from_t = std::remove_reference_t<From>;
        using cast_t = forward_cast_t<From, To>;

        [[nodiscard]] constexpr decltype(auto) operator()(from_t& from) //
            const noexcept
        {
            // c-style cast allow us cast to inaccessible base
            if constexpr(decay_derived<From, To>) return (cast_t)from; // NOLINT
            else return static_cast<cast_t>(from);
        }

        [[nodiscard]] constexpr decltype(auto) operator()(from_t&& from) //
            const noexcept
        {
            // c-style cast allow us cast to inaccessible base
            if constexpr(decay_derived<From, To>) return (cast_t)from; // NOLINT
            else return static_cast<cast_t>(from);
        }
    };

    template<typename From, typename To, typename... Rest>
    inline constexpr forward_cast_fn<From, To, Rest...> forward_cast{};
}

#include "../compilation_config_out.h"
