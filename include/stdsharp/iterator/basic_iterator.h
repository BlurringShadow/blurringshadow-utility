#pragma once

#include "../default_operator.h"
#include "../utility/forward_cast.h"

#include <iterator>

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<class_ T, typename Category = std::random_access_iterator_tag>
        requires requires(T& t) {
            requires std::derived_from<Category, std::input_iterator_tag> ||
                std::derived_from<Category, std::output_iterator_tag>;

            requires dereferenceable<const T&>;
            ++t;
        }
    struct STDSHARP_EBO basic_iterator :
        T,
        default_operator::arithmetic,
        default_operator::arrow,
        default_operator::subscript,
        default_operator::plus_commutative
    {
        using T::T;

        using iterator_category = Category;

        using value_type = std::remove_reference_t<decltype(*std::declval<const T&>())>;

    private:
        static consteval T& val();
        static consteval const T& cval();

        static consteval auto diff_type()
        {
            if constexpr(requires { cval() - cval(); })
                return type_constant<decltype(cval() - cval())>{};
            else return type_constant<void>{};
        }

    public:
        using difference_type = typename decltype(diff_type())::type;

        using default_operator::arithmetic::operator++;
        using T::operator++;

        using default_operator::arithmetic::operator*;
        using T::operator*;

        using default_operator::arithmetic::operator--;

        template<typename U>
            requires requires { --val(); }
        constexpr U& operator--(this U& u) noexcept(noexcept(--val()))
        {
            --forward_cast<U&, T>(u);
            return u;
        }

        using default_operator::subscript::operator[];

        template<typename U>
            requires not_same_as<difference_type, void>
        constexpr decltype(auto) operator[](this const U& u, const difference_type& diff)
            noexcept(noexcept(cval()[diff]))
            requires requires { cval()[diff]; }
        {
            return forward_cast<const U&, T>(u)[diff];
        }

        using default_operator::arithmetic::operator-;

        template<typename U>
            requires not_same_as<difference_type, void>
        constexpr difference_type operator-(this const U& u, decltype(u) u2)
            noexcept(noexcept(cval() - cval()))
        {
            return forward_cast<const U&, T>(u) - forward_cast<const U&, T>(u2);
        }

        template<typename U>
            requires not_same_as<difference_type, void>
        constexpr U& operator-=(this U& u, const difference_type& diff)
            noexcept(noexcept(std::declval<T&>() += -diff))
            requires requires(T& t) { t += -diff; }
        {
            forward_cast<const U&, T>(u) += -diff;
            return u;
        }
    };
}

#include "../compilation_config_out.h"
