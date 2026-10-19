cmake_minimum_required(VERSION 4.0.0)

set(BENCHMARK_ENABLE_TESTING OFF)

include(FetchContent)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark
  GIT_TAG v1.9.5
)
FetchContent_MakeAvailable(benchmark)

add_executable(cyberverse_geometry_benchmarks sphere_benchmark.cpp)

target_link_libraries(
  cyberverse_geometry_benchmarks
  PRIVATE
  cyberverse::geometry::static_lib
  benchmark::benchmark
)
