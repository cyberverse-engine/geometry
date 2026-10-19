cmake_minimum_required(VERSION 4.0.0)

include(FetchContent)

# GLM is header-only; don't build its library, tests, or install rules.
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(glm)

add_library(cyberverse_geometry_static_lib STATIC)
add_library(cyberverse::geometry::static_lib ALIAS cyberverse_geometry_static_lib)

target_include_directories(cyberverse_geometry_static_lib PUBLIC ../src)

# Header files
set(
  HEADER_FILES
  cyberverse/geometry.hpp
  cyberverse/geometry/bounds.hpp
  cyberverse/geometry/intersect.hpp
  cyberverse/geometry/mesh_data.hpp
  cyberverse/geometry/primitives.hpp
  cyberverse/geometry/vertex.hpp
)

# Source files
set(
  SOURCE_FILES
  cyberverse/geometry/bounds.cpp
  cyberverse/geometry/intersect.cpp
  cyberverse/geometry/mesh_data.cpp
  cyberverse/geometry/primitives.cpp
)

target_sources(cyberverse_geometry_static_lib PUBLIC ${HEADER_FILES})
target_sources(cyberverse_geometry_static_lib PRIVATE ${SOURCE_FILES})

target_link_libraries(cyberverse_geometry_static_lib PUBLIC glm::glm)
