#pragma once

namespace stdsharp
{
    enum class expr_req : unsigned char
    {
        ill_formed,
        well_formed,
        no_exception,
    };

    constexpr auto get_expr_req(const bool well_formed, const bool no_exception = false) noexcept
    {
        return well_formed ? //
            no_exception ? expr_req::no_exception : expr_req::well_formed :
            expr_req::ill_formed;
    }

    constexpr bool is_well_formed(const expr_req req) noexcept
    {
        return req >= expr_req::well_formed;
    }

    constexpr bool is_noexcept(const expr_req req) noexcept
    {
        return req >= expr_req::no_exception;
    }
}
