cmake_minimum_required (VERSION 3.14)

project ("coroutine")


# for clangd
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory("src")

# 将源代码添加到此项目的可执行文件。
add_executable (coroutine "main.cc")
target_compile_features(coroutine PRIVATE cxx_std_17)

# 单测
add_subdirectory("test")