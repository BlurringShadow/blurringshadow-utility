#include "stdsharp/functional/bind.h"
#include "stdsharp/type_traits/object.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

void foo()
{
    struct t0 : unique_object
    {
    };

    struct fn
    {
        const t0& operator()(const t0& obj) const { return obj; }
    };

    using invoker = stdsharp::details::forward_bind_traits::front_invoker_fn<fn>;

    const invoker v0{};
    const t0 obj{};
    const indexed_values<const t0&> indexed{obj};

    v0(indexed);
}

SCENARIO("forward bind", "[functional][forward bind]")
{
    struct t0 : unique_object
    {
    };

    t0 obj{};
    const t0 const_obj{};

    THEN("pass const unique object and invoke")
    {
        const auto func =
            forward_bind_front([](const t0& obj) -> const t0& { return obj; }, const_obj);

        const auto back_func =
            forward_bind_back([](int, const t0& obj) -> const t0& { return obj; }, const_obj);

        REQUIRE(&func() == &const_obj);
        REQUIRE(&back_func(1) == &const_obj);
    }

    // THEN("pass rvalue unique object and invoke")
    // {
    //     auto func = forward_bind_front([](t0&& obj) -> t0 { return obj; }, t0{});

    //     auto back_func = forward_bind_back([](int, t0&& obj) -> t0 { return obj; }, t0{});

    //     [[maybe_unused]] auto&& new_obj = cpp_move(func)();
    //     [[maybe_unused]] auto&& new_back_obj = cpp_move(back_func)(1);
    // }

    // THEN("pass lvalue unique object and invoke")
    // {
    //     auto func = forward_bind_front([](t0& obj) -> t0& { return obj; }, obj);

    //     auto back_func = forward_bind_back([](int, t0& obj) -> t0& { return obj; }, obj);

    //     REQUIRE(&func() == &obj);
    //     REQUIRE(&back_func(1) == &obj);
    // }
}
