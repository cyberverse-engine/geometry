cmake_minimum_required(VERSION 4.0.0)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(Catch2)

add_executable(
  cyberverse_geometry_tests
  mesh_data_test.cpp
  bounds_test.cpp
  intersect_test.cpp
  frustum_test.cpp
  primitives_test.cpp
  sphere_test.cpp
)

target_link_libraries(
  cyberverse_geometry_tests
  PRIVATE
  cyberverse::geometry::static_lib
  Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(cyberverse_geometry_tests)
