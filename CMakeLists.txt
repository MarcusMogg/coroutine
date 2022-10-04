cmake_minimum_required (VERSION 3.14)

project ("coroutine")


# for clangd
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# https://github.com/google/sanitizers/issues/721
set(CMAKE_CXX_FLAGS "-D_FORTIFY_SOURCE -m64  -pthread -fcoroutines")

# 将源代码添加到此项目的可执行文件。
add_executable (coroutine "main.cc")
target_compile_features(coroutine PRIVATE cxx_std_20)
target_compile_options(coroutine PRIVATE -fcoroutines)

add_subdirectory("src")
target_link_libraries(coroutine PRIVATE libco)

# 单测
add_subdirectory("test")
target_link_libraries(coroutine PRIVATE test_libco)