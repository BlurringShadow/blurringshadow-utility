#pragma once

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace stdsharp
{
}

#define STDSHARP_TEST_NAMESPACES \
    using namespace std;         \
    using namespace stdsharp
