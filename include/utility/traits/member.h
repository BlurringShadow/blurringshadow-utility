﻿// Created by BlurringShadow at 2021-03-05-下午 11:53

#pragma once

#include "function.h"

namespace blurringshadow::utility::traits
{
    template<typename>
    struct member_traits;

    template<typename T, typename ClassT>
    struct member_traits<T(ClassT::*)> : std::type_identity<T>
    {
        using class_t = ClassT;
    };

    template<auto Ptr>
    struct member_pointer_traits : member_traits<std::decay_t<decltype(Ptr)>>
    {
    };

    template<auto Ptr>
    using member_t = typename member_pointer_traits<Ptr>::type;

    template<typename>
    struct member_function_traits;

    enum class member_ref_qualifier
    {
        none,
        lvalue,
        rvalue
    };

    namespace details
    {
        template<typename R, typename ClassT, typename... Args>
        struct mem_fn_traits : function_traits<R (*)(Args...)>
        {
            using class_t = ClassT;
        };

        template<bool IsConst, bool IsVolatile, member_ref_qualifier RefType>
        struct mem_fn_qualifier_traits
        {
            static constexpr auto is_const = IsConst;
            static constexpr auto is_volatile = IsVolatile;
            static constexpr member_ref_qualifier ref_type = RefType;
        };
    }

#define UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS(is_const, is_volatile, ref_type, qualifiers)      \
    template<typename R, typename ClassT, typename... Args>                                     \
    struct member_function_traits<R (ClassT::*)(Args...)(qualifiers)> :                         \
        details::mem_fn_traits<R, ClassT, Args...>,                                             \
        details::mem_fn_qualifier_traits<is_const, is_volatile, member_ref_qualifier::ref_type> \
    {                                                                                           \
    };

#define UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_CONST_PACK(is_volatile, ref_type, qualifiers) \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS(true, is_volatile, ref_type, const qualifiers)    \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS(false, is_volatile, ref_type, qualifiers)

#define UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(ref_type, qualifiers)          \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_CONST_PACK(true, ref_type, volatile(qualifiers)) \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_CONST_PACK(false, ref_type, qualifiers)

#define UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_REF_PACK             \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(none, )    \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(lvalue, &) \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(rvalue, &&)

    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_REF_PACK

#undef UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_REF_PACK
#undef UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK
#undef UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_CONST_PACK
#undef UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS

    template<auto Ptr>
    struct member_function_pointer_traits : member_function_traits<std::decay_t<decltype(Ptr)>>
    {
    };

    template<typename T, typename ClassT>
    concept member_of = std::same_as<typename member_traits<T>::class_t, ClassT>;

    template<typename T, typename ClassT>
    concept member_func_of = std::is_member_pointer_v<T> &&
        std::same_as<typename member_function_traits<T>::class_t, ClassT>;
}
