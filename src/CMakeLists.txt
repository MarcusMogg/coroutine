file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE CC_FILES "*.cc" "*.cpp")

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_library (libco STATIC ${HEADER_FILES} ${CC_FILES})

target_compile_features(libco PRIVATE cxx_std_17)

target_include_directories(libco
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
)
