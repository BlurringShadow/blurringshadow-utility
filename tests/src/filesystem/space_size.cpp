#include "stdsharp/filesystem/space_size.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;
using namespace stdsharp::filesystem;

SCENARIO("space size", "[filesystem][space size]")
{
    using bytes = stdsharp::filesystem::bytes;

    STATIC_REQUIRE(default_initializable<bytes>);
    STATIC_REQUIRE(invocable<plus<>, bytes, bytes>);
    STATIC_REQUIRE(invocable<minus<>, bytes, bytes>);

    STATIC_REQUIRE((1_bits + 1_bits) == 2_bits);

#if !(                                               \
    defined(__clang_analyzer__) && defined(_MSC_VER) \
) // TODO: https://github.com/llvm/llvm-project/issues/93271
    REQUIRE(format("{}", 42_KB) == "42KB");
    REQUIRE(format("{:-<10.2}", 1.2_GB) == "1.2GB-----");
    REQUIRE(format("{:->10.2}", 1.2_GB) == "-----1.2GB");
    REQUIRE(format("{:-^10.2}", 1.2_GB) == "--1.2GB---");
    REQUIRE(format("{:-^14.2}", 1.2_GB) == "----1.2GB-----");
    REQUIRE(format("{:-^17.2{1}}", 1.2_GB, "GigaBytes") == "--1.2GigaBytes---");

#endif
};
