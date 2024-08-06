#pragma once

#include "type_traits/type.h"

namespace stdsharp
{
    template<typename T>
    using rvalue_ref_t = ::stdsharp::override_ref_t<T, ::stdsharp::ref_qualifier::rvalue>;
}

#define cpp_forward(...) static_cast<decltype(__VA_ARGS__)>(__VA_ARGS__)
#define cpp_move(...) static_cast<::stdsharp::rvalue_ref_t<decltype(__VA_ARGS__)>>(__VA_ARGS__)
#define cpp_is_constexpr(...) requires { noexcept((__VA_ARGS__, true)); }
