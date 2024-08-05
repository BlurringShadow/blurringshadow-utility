#
# Project details
#
project(${CMAKE_PROJECT_NAME}Tests LANGUAGES CXX)

verbose_message("Adding tests under ${CMAKE_PROJECT_NAME}...")

find_package(Catch2 CONFIG REQUIRED)
include(Catch)

#
# Set the sources for the unit tests and add the executable(s)
#
set(src
    src/algorithm/algorithm.cpp
    src/bitset/bitset.cpp
    src/containers/actions.cpp
    src/containers/concepts.cpp
    src/filesystem/space_size.cpp
    src/functional/forward_bind.cpp
    src/functional/invocables.cpp
    src/functional/pipeable.cpp
    src/functional/sequenced_invocables.cpp
    src/memory/allocation_value.cpp
    src/memory/box.cpp
    src/memory/composed_allocator.cpp
    src/memory/fixed_single_allocator.cpp
    src/memory/launder_iterator.cpp
    src/memory/pointer_traits.cpp
    src/memory/soo.cpp
    src/random/random.cpp
    src/type_traits/indexed_traits.cpp
    src/type_traits/int_type_1.cpp
    src/type_traits/int_type_2.cpp
    src/type_traits/member.cpp
    src/type_traits/type_sequence.cpp
    src/type_traits/type.cpp
    src/type_traits/value_sequence.cpp
    src/utility/adl_proof.cpp
    src/utility/dispatcher.cpp
    src/utility/forward_cast.cpp
    src/utility/utility.cpp
    src/utility/value_wrapper.cpp
    src/default_operator.cpp
    src/lazy.cpp
    src/pattern_match.cpp
    src/synchronizer.cpp
)

config_lib(${PROJECT_NAME}Lib INTERFACE)

target_link_libraries(
    ${PROJECT_NAME}Lib
    INTERFACE stdsharp::stdsharp Catch2::Catch2WithMain
)

config_exe(${PROJECT_NAME} EXE_SRC ${src})

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC
        $<$<CXX_COMPILER_ID:Clang>:-Qunused-arguments
        -ftemplate-backtrace-limit=0>
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES COMPILE_WARNING_AS_ERROR ON EXPORT_COMPILE_COMMANDS ON
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}Lib)

target_clang_sanitizer(${PROJECT_NAME} PUBLIC SANITIZER address undefined)

#
# Configure clang-tidy
#
option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)

if(${ENABLE_CLANG_TIDY})
    message(STATUS "clang-tidy is enabled")
    target_clang_tidy(${PROJECT_NAME})
endif()

#
# Configure code coverage
#
set(CODE_COVERAGE_EXT CACHE STRING "Enable code coverage")

if(CODE_COVERAGE_EXT)
    target_llvm_coverage(${PROJECT_NAME} FORMAT ${CODE_COVERAGE_EXT})
endif()

catch_discover_tests(${PROJECT_NAME})

target_cmake_format(
  ${PROJECT_NAME}
  SRC
    "${CMAKE_SOURCE_DIR}/CMakeLists.txt"
    "${CMAKE_SOURCE_DIR}/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
