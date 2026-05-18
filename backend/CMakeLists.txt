cmake_minimum_required(VERSION 3.16)
project(graph_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(graph_engine
    src/main.cpp
    src/graph/graph_impl_final.cpp
    src/probabilistic/hll.cpp
    src/communities/dsu.cpp
    src/string_matching/trie.cpp
    src/string_matching/aho_corasick.cpp
)

target_include_directories(graph_engine PRIVATE
    src/graph
    src/probabilistic
    src/communities
    src/string_matching
)

if (WIN32)
    target_link_libraries(graph_engine PRIVATE ws2_32)
else()
    target_link_libraries(graph_engine PRIVATE pthread)
endif()

if (MSVC)
    target_compile_options(graph_engine PRIVATE /W4)
else()
    target_compile_options(graph_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if (BUILD_TESTING)
    add_executable(graph_smoke_tests
        tests/graph_smoke.cpp
        src/graph/graph_impl_final.cpp
        src/probabilistic/hll.cpp
        src/communities/dsu.cpp
        src/string_matching/trie.cpp
        src/string_matching/aho_corasick.cpp
    )
    target_include_directories(graph_smoke_tests PRIVATE
        src/graph
        src/probabilistic
        src/communities
        src/string_matching
    )
    if (MSVC)
        target_compile_options(graph_smoke_tests PRIVATE /W4)
    else()
        target_compile_options(graph_smoke_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME graph_smoke_tests COMMAND graph_smoke_tests)
endif()
