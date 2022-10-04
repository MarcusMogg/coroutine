file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE CC_FILES "*.cc" "*.cpp")


add_library (test_libco STATIC ${HEADER_FILES} ${CC_FILES})

target_compile_features(test_libco PRIVATE cxx_std_20)

target_link_libraries(test_libco PRIVATE libco)

target_include_directories(test_libco
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
)
