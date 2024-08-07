#pragma once

#include "type_traits/type.h"

namespace stdsharp
{
    template<typename T>
    using cast_to_rvalue_t = override_ref_t<T, ref_qualifier::rvalue>;
}

#define cpp_forward(...) static_cast<decltype(__VA_ARGS__)>(__VA_ARGS__)
#define cpp_move(...) static_cast<::stdsharp::cast_to_rvalue_t<decltype(__VA_ARGS__)>>(__VA_ARGS__)
#define cpp_is_constexpr(...) requires { noexcept((__VA_ARGS__, true)); }
