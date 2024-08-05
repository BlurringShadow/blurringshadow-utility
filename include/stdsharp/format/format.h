#pragma once

#include "../functional/sequenced_invocables.h"

#include <ctre.hpp>

#include <format>
#include <numeric>
#include <optional>
#include <ranges>
#include <variant>

namespace stdsharp::details
{
    template<typename CharT>
    constexpr std::conditional_t<true, void, CharT> space_char;

    template<>
    inline constexpr auto space_char<char> = ' ';

    template<>
    inline constexpr auto space_char<wchar_t> = L' ';

    template<typename CharT>
    constexpr std::conditional_t<true, void, CharT> zero_num_char;

    template<>
    inline constexpr auto zero_num_char<char> = '0';

    template<>
    inline constexpr auto zero_num_char<wchar_t> = L'0';

    template<typename CharT>
    constexpr std::conditional_t<true, void, CharT> fill_and_align_regex;

    template<>
    inline constexpr ctll::fixed_string fill_and_align_regex<char>{"([^<^>])([<^>])"};

    template<>
    inline constexpr ctll::fixed_string fill_and_align_regex<wchar_t>{L"([^<^>])([<^>])"};

    template<typename CharT>
    constexpr std::conditional_t<true, void, CharT> nested_fmt_regex;

    template<>
    inline constexpr ctll::fixed_string nested_fmt_regex<char>{R"(\{(\d*)\})"};

    template<>
    inline constexpr ctll::fixed_string nested_fmt_regex<wchar_t>{LR"(\{(\d*)\})"};

    template<typename CharT>
    constexpr std::conditional_t<true, void, CharT> precision_dot_regex;

    template<>
    inline constexpr ctll::fixed_string precision_dot_regex<char>{"\\."};

    template<>
    inline constexpr ctll::fixed_string precision_dot_regex<wchar_t>{L"\\."};

    template<typename CharT>
    constexpr std::conditional_t<true, void, CharT> int_regex;

    template<>
    inline constexpr ctll::fixed_string int_regex<char>{R"((\d*))"};

    template<>
    inline constexpr ctll::fixed_string int_regex<wchar_t>{LR"((\d*))"};

    template<typename CharT>
    constexpr std::conditional_t<true, void, CharT> locale_regex;

    template<>
    inline constexpr ctll::fixed_string locale_regex<char>{"L"};

    template<>
    inline constexpr ctll::fixed_string locale_regex<wchar_t>{L"L"};
}

namespace stdsharp
{

    template<typename CharT = char>
    struct make_format_args_fn;

    template<>
    struct make_format_args_fn<char>
    {
        template<typename... Args>
        [[nodiscard]] constexpr auto operator()(Args&&... args) const
        {
            return std::make_format_args(std::forward<Args>(args)...);
        }
    };

    template<>
    struct make_format_args_fn<wchar_t>
    {
        template<typename... Args>
        [[nodiscard]] constexpr auto operator()(Args&&... args) const
        {
            return std::make_wformat_args(std::forward<Args>(args)...);
        }
    };

    template<typename CharT = char>
    inline constexpr make_format_args_fn<CharT> make_format_args{};

    template<typename CharT>
    inline constexpr auto fill_and_align_regex = details::fill_and_align_regex<CharT>;

    template<typename CharT>
    inline constexpr auto nested_fmt_regex = details::nested_fmt_regex<CharT>;

    template<typename CharT>
    inline constexpr auto precision_dot_regex = details::precision_dot_regex<CharT>;

    template<typename CharT>
    inline constexpr auto int_regex = details::int_regex<CharT>;

    template<typename CharT>
    inline constexpr auto locale_regex = details::locale_regex<CharT>;

    inline constexpr struct visit_fmt_arg_fn
    {
        template<typename Visitor, typename OutputIt, typename CharT>
            requires requires(const std::
                                  basic_format_arg<std::basic_format_context<OutputIt, CharT>>::
                                      handle handle) {
                requires std::invocable<Visitor, const bool&>;
                requires std::invocable<Visitor, const CharT&>;
                requires std::invocable<Visitor, const int&>;
                requires std::invocable<Visitor, const unsigned&>;
                requires std::invocable<Visitor, const long long&>;
                requires std::invocable<Visitor, const unsigned long long&>;
                requires std::invocable<Visitor, const float&>;
                requires std::invocable<Visitor, const double&>;
                requires std::invocable<Visitor, const long double&>;
                requires std::invocable<Visitor, const CharT*>;
                requires std::invocable<Visitor, const std::basic_string_view<CharT*>>;
                requires std::invocable<Visitor, const void*>;
                requires std::invocable<Visitor, decltype(handle)>;

                requires all_same<
                    std::invoke_result_t<Visitor, const bool&>,
                    std::invoke_result_t<Visitor, const CharT&>,
                    std::invoke_result_t<Visitor, const int&>,
                    std::invoke_result_t<Visitor, const unsigned&>,
                    std::invoke_result_t<Visitor, const long long&>,
                    std::invoke_result_t<Visitor, const unsigned long long&>,
                    std::invoke_result_t<Visitor, const float&>,
                    std::invoke_result_t<Visitor, const double&>,
                    std::invoke_result_t<Visitor, const long double&>,
                    std::invoke_result_t<Visitor, const CharT*>,
                    std::invoke_result_t<Visitor, const std::basic_string_view<CharT*>>,
                    std::invoke_result_t<Visitor, const void*>,
                    std::invoke_result_t<Visitor, decltype(handle)>>;
            }
        [[nodiscard]] constexpr decltype(auto) operator()(
            const std::basic_format_context<OutputIt, CharT>& ctx,
            const std::size_t id,
            Visitor visitor
        ) const
        {
#if __cpp_lib_format >= 202306L
            return ctx.arg(id).visit(visitor);
#else
            return std::visit_format_arg(cpp_move(visitor), ctx.arg(id));
#endif
        }
    } visit_fmt_arg{};

    template<typename CharT>
    [[noreturn]] void parse_assert(const std::basic_format_parse_context<CharT>& ctx)
    {
        const auto begin = ctx.begin();
        throw std::format_error{
            std::format(
                "invalid format:\n \"{}\"\n{}",
                std::basic_string_view{begin, ctx.end()},
                std::format(
                    "{}^ Unexpected character here",
                    std::views::repeat(' ', begin - ctx.begin())
                )
            ) //
        };
    }

    template<typename CharT, std::predicate<CharT> Predicate>
    constexpr void
        parse_validate(const std::basic_format_parse_context<CharT>& ctx, Predicate predicate)
    {
        if(const auto begin = ctx.begin(); begin == ctx.end() || !predicate(*begin))
            parse_assert(ctx, begin);
    }

    template<typename CharT>
    constexpr void parse_not_end_assert(const std::basic_format_parse_context<CharT>& ctx)
    {
        parse_validate(ctx, [](const auto) { return true; });
    }

    template<typename CharT>
    constexpr void parse_end_assert(const std::basic_format_parse_context<CharT>& ctx)
    {
        const auto begin = ctx.begin();

        if(begin == ctx.end() || *begin == '}') return;

        throw std::format_error{
            std::format(
                "invalid format: \"{}\"\nEnd of string expected",
                std::basic_string_view{begin, ctx.end()}
            ) //
        };
    }

    enum class format_align_t : std::uint8_t
    {
        none,
        left,
        right,
        center
    };

    [[nodiscard]] constexpr format_align_t get_format_align(const char align_char)
    {
        switch(align_char)
        {
        case '<': return format_align_t::left;
        case '>': return format_align_t::right;
        case '^': return format_align_t::center;
        default: throw std::format_error{"invalid align specifier"};
        }
    }

    [[nodiscard]] constexpr format_align_t get_format_align(const wchar_t align_char)
    {
        switch(align_char)
        {
        case L'<': return format_align_t::left;
        case L'>': return format_align_t::right;
        case L'^': return format_align_t::center;
        default: throw std::format_error{"invalid align specifier"};
        }
    }

    template<typename CharT>
        requires requires { details::space_char<CharT>; }
    struct fill_and_align_spec
    {
        format_align_t align = format_align_t::none;
        CharT fill = details::space_char<CharT>;
    };

    template<typename Rng, typename CharT = std::remove_cvref_t<std::ranges::range_value_t<Rng>>>
        requires requires { details::zero_num_char<CharT>; }
    [[nodiscard]] constexpr auto parse_decimal_integer(const Rng& rng) noexcept
    {
        const auto& num_rng = std::views::transform(
            rng,
            [](const CharT& char_v) { return char_v - details::zero_num_char<CharT>; }
        );

        return std::accumulate(
            num_rng.begin(),
            num_rng.end(),
            uintmax_t{0},
            [](const auto& v, const auto& c) { return v * 10 + c; } // NOLINT(*-magic-numbers)
        );
    };

    template<typename CharT, auto Regex = details::fill_and_align_regex<CharT>>
    [[nodiscard]] constexpr fill_and_align_spec<CharT>
        parse_fill_and_align_spec(std::basic_format_parse_context<CharT>& ctx)
    {
        const auto& [whole, fill, align] = ctre::starts_with<Regex>(ctx);

        if(!whole) return {};

        fill_and_align_spec<CharT> spec;

        if(fill) spec.fill = *fill.begin();

        spec.align = get_format_align(*align.begin());

        ctx.advance_to(whole.end());

        return spec;
    }

    struct nested_fmt_spec
    {
        std::size_t id;
    };

    template<typename CharT, auto Regex = details::nested_fmt_regex<CharT>>
    [[nodiscard]] constexpr std::optional<nested_fmt_spec>
        parse_nested_spec(std::basic_format_parse_context<CharT>& ctx)
    {
        const auto& [whole, ref] = ctre::starts_with<Regex>(ctx);

        if(!whole) return {};

        const auto int_v = std::ranges::empty(ref) ? //
            ctx.next_arg_id() :
            static_cast<std::size_t>(parse_decimal_integer(ref));

        ctx.advance_to(whole.end());

        return nested_fmt_spec{int_v};
    }

    template<std::unsigned_integral IntType = uintmax_t>
    using uint_nested_maybe_spec = std::variant<std::monostate, IntType, nested_fmt_spec>;

    template<
        typename IntType = uintmax_t,
        typename CharT,
        auto Regex = details::precision_dot_regex<CharT>>
    constexpr uint_nested_maybe_spec<IntType>
        parse_uint_maybe_nested_spec(std::basic_format_parse_context<CharT>& ctx)
    {
        if(const auto& [whole, int_v] = ctre::starts_with<details::int_regex<CharT>>(ctx); whole)
        {
            ctx.advance_to(whole.end());
            return static_cast<IntType>(parse_decimal_integer(int_v));
        }

        if(const auto& nested = parse_nested_spec(ctx); nested) return *nested;

        return std::monostate{};
    }

    template<
        typename IntType = uintmax_t,
        typename CharT,
        auto Regex = details::precision_dot_regex<CharT>>
    constexpr uint_nested_maybe_spec<IntType>
        parse_precision_spec(std::basic_format_parse_context<CharT>& ctx)
    {
        const auto& dot = ctre::starts_with<Regex>(ctx);

        if(!dot) return {};

        ctx.advance_to(dot.end());

        auto&& spec = parse_uint_maybe_nested_spec<IntType>(ctx);

        if(spec.index() == 0) throw std::format_error{"specify precision without valid number"};

        return spec;
    }

    template<typename CharT, auto Regex = details::locale_regex<CharT>>
    constexpr bool parse_locale_spec(std::basic_format_parse_context<CharT>& ctx)
    {
        const auto& use_locale = ctre::starts_with<Regex>(ctx);

        if(!use_locale) return false;

        ctx.advance_to(use_locale.end());

        return true;
    }
}

namespace stdsharp::details
{
    template<std::unsigned_integral IntType>
    struct fmt_int_cast_fn
    {
        constexpr auto operator()(const std::integral auto& value)
        {
            return value > 0 ? static_cast<IntType>(value) : throw std::format_error{"invalid num"};
        }

        constexpr auto operator()(const std::unsigned_integral auto& value)
        {
            return static_cast<IntType>(value);
        }
    };

    template<std::unsigned_integral IntType>
    struct fmt_int_invalid_cast_fn
    {
        [[noreturn]] constexpr IntType operator()(const auto& /*unused*/)
        {
            throw std::format_error{"invalid num"};
        }
    };
}

namespace stdsharp
{
    template<std::unsigned_integral IntType, typename OutputIt, typename CharT>
    static constexpr std::optional<IntType> get_maybe_nested_uint(
        const ::stdsharp::uint_nested_maybe_spec<IntType>& spec,
        const std::basic_format_context<OutputIt, CharT>& ctx
    )
    {
        switch(spec.index())
        {
        case 0: return {};

        case 1: return get<1>(spec);

        case 2:
            return ::stdsharp::visit_fmt_arg(
                ctx,
                get<2>(spec).id,
                sequenced_invocables<
                    details::fmt_int_cast_fn<IntType>,
                    details::fmt_int_invalid_cast_fn<IntType>>{}
            );
        }

        std::unreachable();

        return {};
    }
}
