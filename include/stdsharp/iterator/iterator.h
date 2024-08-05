#pragma once

#include "basic_iterator.h" // IWYU pragma: export

namespace stdsharp
{
    template<std::indirectly_readable T>
    using iter_const_reference_t =
#if __cpp_lib_ranges_as_const >= 202207L
        std::iter_const_reference_t<T>
#else
        std::common_reference_t<const std::iter_value_t<T>&&, std::iter_reference_t<T>>
#endif
        ;

    template<typename T>
    concept weakly_decrementable = std::movable<T> && requires(T i) {
        typename std::iter_difference_t<T>;
        requires std::signed_integral<std::iter_difference_t<T>>;
        { --i } -> std::same_as<T&>;
        i--;
    };

    template<typename T>
    concept decrementable = std::regular<T> && weakly_decrementable<T> && requires(T i) {
        { i-- } -> std::same_as<T>;
    };
}
