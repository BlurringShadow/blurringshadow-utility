#pragma once

#include "../default_operator.h"

#include <iterator>

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<
        typename ValueType,
        typename DifferenceType,
        typename Category = std::random_access_iterator_tag>
    struct STDSHARP_EBO basic_iterator :
        default_operator::arithmetic,
        default_operator::arrow,
        default_operator::subscript,
        default_operator::plus_commutative
    {
        using iterator_category = Category;

        using value_type = ValueType;

        using difference_type = DifferenceType;

        using default_operator::arithmetic::operator++;
        using default_operator::arithmetic::operator*;
        using default_operator::arithmetic::operator--;
        using default_operator::subscript::operator[];
        using default_operator::arithmetic::operator-;

        constexpr decltype(auto) operator-=(this auto& u, const auto& diff)
            noexcept(noexcept(u += -diff))
            requires requires { u += -diff; }
        {
            return u += -diff;
        }
    };
}

#include "../compilation_config_out.h"
