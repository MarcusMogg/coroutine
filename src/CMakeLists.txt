file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE CC_FILES "*.cc" "*.cpp")


add_library (libco STATIC ${HEADER_FILES} ${CC_FILES})

target_compile_features(libco PRIVATE cxx_std_20)

target_include_directories(libco
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
)
