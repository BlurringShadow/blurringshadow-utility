#pragma once

#include "../namespace_alias.h"

#include <concepts>
#include <type_traits>

namespace stdsharp
{
    template<typename T, typename... U>
    concept same_as_any = (std::same_as<T, U> || ...);

    template<typename T, typename... U>
    concept all_same = (std::same_as<T, U> && ...);

    template<typename T, typename U>
    concept not_same_as = !std::same_as<T, U>;

    template<typename T, typename U>
    concept decay_same_as = std::same_as<std::decay_t<T>, U>;

    template<typename T>
    concept enum_ = std::is_enum_v<T>;

    template<typename T>
    concept reference = std::is_reference_v<T>;

    template<typename T>
    concept lvalue_ref = std::is_lvalue_reference_v<T>;

    template<typename T>
    concept rvalue_ref = std::is_rvalue_reference_v<T>;

    template<typename T>
    concept const_ = std::is_const_v<T>;

    template<typename T>
    concept non_const = !const_<T>;

    template<typename T>
    concept const_lvalue_ref = std::is_lvalue_reference_v<T> && const_<std::remove_reference_t<T>>;

    template<typename T>
    concept volatile_ = std::is_volatile_v<T>;

    template<typename T>
    concept const_volatile = const_<T> && volatile_<T>;

    template<typename T>
    concept abstract = std::is_abstract_v<T>;

    template<typename T>
    concept aggregate = std::is_aggregate_v<T>;

    template<typename T>
    concept void_ = std::is_void_v<T>;

    template<typename T>
    concept character = same_as_any<T, char, char8_t, char16_t, char32_t, wchar_t>;

    template<typename T>
    concept arithmetic = std::is_arithmetic_v<T>;

    template<typename T>
    concept empty_type = std::is_empty_v<T>;

    template<typename T>
    concept carray = std::is_array_v<T>;

    template<typename B, typename D>
    concept base_of = std::is_base_of_v<B, D>;

    template<typename D, typename B>
    concept decay_derived = base_of<std::remove_reference_t<B>, std::remove_reference_t<D>>;

    template<typename D, typename B>
    concept not_decay_derived = !decay_derived<D, B>;

    template<typename T>
    concept class_ = std::is_class_v<T>;

    template<typename T>
    concept function = std::is_function_v<T>;

    template<typename T>
    concept pointer = std::is_pointer_v<T>;

    template<typename T>
    concept fundamental = std::is_fundamental_v<T>;

    template<typename T>
    concept scalar = std::is_scalar_v<T>;

    template<typename T>
    concept object = std::is_object_v<T>;

    template<typename T>
    concept compound = std::is_compound_v<T>;

    template<typename T>
    concept member_object_pointer = std::is_member_object_pointer_v<T>;

    template<typename T>
    concept member_function_pointer = std::is_member_function_pointer_v<T>;

    template<typename T>
    concept member_pointer = std::is_member_pointer_v<T>;

    template<typename T>
    concept polymorphic = std::is_polymorphic_v<T>;

    template<typename T>
    concept final = std::is_final_v<T>;

    template<typename T>
    concept nttp_able = requires {
        []<T...> {}();
    };

    template<typename T, typename U>
    concept const_aligned = (const_<T> == const_<U>);

    template<typename T, typename U>
    concept ref_aligned = (lvalue_ref<T> == lvalue_ref<U>) && (rvalue_ref<T> == rvalue_ref<U>);

    template<typename T, typename U>
    concept const_ref_aligned = const_aligned<T, U> && ref_aligned<T, U>;
}
