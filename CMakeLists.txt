cmake_minimum_required(VERSION 3.3)
project(libfive)

set (PKG_CONFIG_EXECUTABLE C:/libs/pkg/bin/pkg-config.exe)
set (PKG_CONFIG_PATH C:/libs/pkg)


# Properly distinguish between Apple and upstream Clang
cmake_policy(SET CMP0025 NEW)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

################################################################################

if(MSVC)
    set(CMAKE_CXX_FLAGS "-Wall -bigobj ")
    set(CMAKE_CXX_FLAGS_RELEASE  "-DRELEASE -DEIGEN_NO_DEBUG")
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -fPIC -pedantic -Werror=switch -march=native")
    set(CMAKE_CXX_FLAGS_RELEASE  "-DRELEASE -DEIGEN_NO_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG    "-O0")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

# Work around an issue with Boost::Interval on OpenBSD
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "OpenBSD")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_ISOC99")
endif()

################################################################################
# Find all packages here at the top level so we can print debugging info
if(NOT MSVC)
    find_package(Qt5Core)
    find_package(Boost REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GUILE guile-2.2>=2.2.1)
    pkg_check_modules(EIGEN REQUIRED eigen3>=3.2.92)
else()
    find_package(eigen3 REQUIRED)
    find_package(boost REQUIRED)
    find_package(PNG REQUIRED)
endif()

if(APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GUILE guile-2.2)
    pkg_check_modules(EIGEN REQUIRED eigen3)
endif(APPLE)


if (UNIX AND NOT(APPLE))
    find_package(Threads REQUIRED)
endif(UNIX AND NOT(APPLE))

################################################################################
# Inform the user which subsystems will be built and any missing deps
message("Checking dependencies:")
message("  libfive: ✓")

if (GUILE_FOUND)
    message("  libfive-guile:     ✓")

    # Sadly, this is a global setting (there's no target_link_directories)
    link_directories(${GUILE_LIBRARY_DIRS})
else ()
    message("  libfive-guile:     ✘   (needs Guile 2.2 or later)")
endif()

if (Qt5Core_FOUND AND GUILE_FOUND)
    message("  Studio:       ✓")
else ()
    if (Qt5Core_FOUND)
        message("  Studio:       ✘   (needs Guile 2.2 or later)")
    elseif (GUILE_FOUND)
        message("  Studio:       ✘   (Qt 5.7 or later)")
    else()
        message("  Studio:       ✘   (needs Guile 2.2 or later and Qt 5.7 or later)")
    endif()
endif()

################################################################################

# Set a flag to detect the case where users run CMake in the wrong directory
set(LIBFIVE_BUILD_FROM_ROOT true)

# Always build the kernel and test suite
add_subdirectory(libfive)

if(GUILE_FOUND AND Qt5Core_FOUND)
    add_subdirectory(studio)
endif(GUILE_FOUND AND Qt5Core_FOUND)

if(MSVC)
    target_compile_definitions(five PUBLIC
        D_SCL_SECURE_NO_WARNINGS
        _USE_MATH_DEFINES)
endif()
