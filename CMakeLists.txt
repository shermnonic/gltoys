cmake_minimum_required(VERSION 3.28)

project(gltoys 
    VERSION 0.3.0
    DESCRIPTION "OpenGL toys by 386dx25"
    LANGUAGES CXX)

# Use vcpkg manifest and toolchain
if(DEFINED ENV{VCPKG_ROOT})
    include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    include("${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
else()
    message(FATAL_ERROR "Vcpkg not found")
endif()

set(CMAKE_CXX_STANDARD 20)

set(BUILD_GLTOYS_APPS true CACHE BOOL "Build gltoys applications.")
set(BUILD_TOYLIB_TEST_APPS true CACHE BOOL "Build toylib test applications for development.")
set(BUILD_SHARED_LIBRARY false CACHE BOOL "Buid shared library.")

add_subdirectory(source)
