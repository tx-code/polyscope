# ## Configure the compiler
# This is a basic, decent setup that should do something sane on most compilers

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # using Clang (linux or apple) or GCC
    message("Using clang/gcc compiler flags")
    SET(CMAKE_CXX_STANDARD 11)
    SET(BASE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
    SET(DISABLED_WARNINGS " -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-deprecated-declarations -Wno-missing-braces -Wno-unused-label -Wno-register -Wno-sign-compare -Wno-unknown-pragmas -Wno-unused-result -Wno-narrowing -Wno-unused-but-set-variable -Wno-unused-lambda-capture -Wno-unused-local-typedefs")
    SET(TRACE_INCLUDES " -H -Wno-error=unused-command-line-argument")

    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        message("Setting clang-specific options")
        SET(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -ferror-limit=3 -Wfatal-errors -fcolor-diagnostics")
        SET(CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address -fno-limit-debug-info")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message("Setting gcc-specific options")
        SET(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -fmax-errors=5")
        SET(CMAKE_CXX_FLAGS_DEBUG "-g3")
        SET(DISABLED_WARNINGS "${DISABLED_WARNINGS} -Wno-maybe-uninitialized -Wno-format-zero-length -Wno-unused-but-set-parameter -Wno-unused-but-set-variable")
    endif()

    SET(CMAKE_CXX_FLAGS "${BASE_CXX_FLAGS} ${DISABLED_WARNINGS} -std=c++11")

    include(CheckCXXCompilerFlag)
    unset(COMPILER_SUPPORTS_MARCH_NATIVE CACHE)
    CHECK_CXX_COMPILER_FLAG(-march=native COMPILER_SUPPORTS_MARCH_NATIVE)

    if(COMPILER_SUPPORTS_MARCH_NATIVE)
        SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG -std=c++11")
    else()
        SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -mcpu=apple-m1 -DNDEBUG -std=c++11") # Apple M1
    endif()

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++
    message("Using Visual Studio compiler flags")
    set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} /W4")
    set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} /MP") # parallel build
    SET(DISABLED_WARNINGS "${DISABLED_WARNINGS} /wd\"4267\"") # ignore conversion to smaller type (fires more aggressively than the gcc version, which is annoying)
    SET(DISABLED_WARNINGS "${DISABLED_WARNINGS} /wd\"4244\"") # ignore conversion to smaller type (fires more aggressively than the gcc version, which is annoying)
    SET(DISABLED_WARNINGS "${DISABLED_WARNINGS} /wd\"4305\"") # ignore truncation on initialization
    SET(CMAKE_CXX_FLAGS "${BASE_CXX_FLAGS} ${DISABLED_WARNINGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

    add_definitions(/D "_CRT_SECURE_NO_WARNINGS")
    add_definitions(-DNOMINMAX)
    add_definitions(-D_USE_MATH_DEFINES)
else()
    # unrecognized
    message(FATAL_ERROR "Unrecognized compiler [${CMAKE_CXX_COMPILER_ID}]")
endif()

# == Build our project stuff
set(SRCS
    src/utils.cpp
    src/main.cpp
)

add_executable(signed-heat-demo "${SRCS}")
target_include_directories(signed-heat-demo PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

find_package(args REQUIRED)
find_package(imgui CONFIG REQUIRED)
target_link_libraries(signed-heat-demo geometry-central polyscope imgui::imgui taywee::args)

# Copy data directory to bin directory after build
add_custom_command(TARGET signed-heat-demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/data"
    "${CMAKE_CURRENT_BINARY_DIR}/data"
    VERBATIM
)

message(STATUS "Source data dir: ${CMAKE_CURRENT_SOURCE_DIR}/data")
message(STATUS "Target data dir: ${CMAKE_CURRENT_BINARY_DIR}/data")
