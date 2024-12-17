cmake_minimum_required(VERSION 3.15)
project(stl LANGUAGES CXX)

# Define the header-only library
add_library(stl INTERFACE)
target_include_directories(stl INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Install headers
install(DIRECTORY include/ DESTINATION include)

# Install the library and export the targets
install(TARGETS stl EXPORT stl_targets)
install(EXPORT stl_targets
        NAMESPACE stl::
        DESTINATION lib/cmake/stl)

# Generate a version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/stl_version.cmake"
    VERSION 1.0
    COMPATIBILITY AnyNewerVersion
)

# Install Config and Version files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/stl_version.cmake"
    DESTINATION lib/cmake/stl
)
