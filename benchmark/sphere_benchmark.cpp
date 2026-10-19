#include <benchmark/benchmark.h>

#include "cyberverse/geometry/primitives.hpp"

using namespace cyberverse::geometry;

namespace {

void benchmark_sphere(benchmark::State& state, const sphere_options& opts) {
  for (auto _ : state) {
    const mesh_data mesh = create_sphere(1.0f, opts);
    benchmark::DoNotOptimize(mesh.vertex_count());
    benchmark::DoNotOptimize(mesh.triangle_count());
    benchmark::ClobberMemory();
  }

  const mesh_data mesh = create_sphere(1.0f, opts);
  state.counters["vertices"] = static_cast<double>(mesh.vertex_count());
  state.counters["triangles"] = static_cast<double>(mesh.triangle_count());
}

} // namespace

static void BM_SphereUV_8x12(benchmark::State& state) {
  sphere_options opts;
  opts.kind = sphere_kind::uv;
  opts.rings = 8;
  opts.segments = 12;
  benchmark_sphere(state, opts);
}

static void BM_SphereUV_32x64(benchmark::State& state) {
  sphere_options opts;
  opts.kind = sphere_kind::uv;
  opts.rings = 32;
  opts.segments = 64;
  benchmark_sphere(state, opts);
}

static void BM_SphereIco_2(benchmark::State& state) {
  sphere_options opts;
  opts.kind = sphere_kind::ico;
  opts.ico_subdivisions = 2;
  benchmark_sphere(state, opts);
}

static void BM_SphereIco_4(benchmark::State& state) {
  sphere_options opts;
  opts.kind = sphere_kind::ico;
  opts.ico_subdivisions = 4;
  benchmark_sphere(state, opts);
}

static void BM_SphereIco_6(benchmark::State& state) {
  sphere_options opts;
  opts.kind = sphere_kind::ico;
  opts.ico_subdivisions = 6;
  benchmark_sphere(state, opts);
}

static void BM_SphereQuad_8(benchmark::State& state) {
  sphere_options opts;
  opts.kind = sphere_kind::quad;
  opts.quad_resolution = 8;
  benchmark_sphere(state, opts);
}

static void BM_SphereQuad_16(benchmark::State& state) {
  sphere_options opts;
  opts.kind = sphere_kind::quad;
  opts.quad_resolution = 16;
  benchmark_sphere(state, opts);
}

static void BM_SphereQuad_32(benchmark::State& state) {
  sphere_options opts;
  opts.kind = sphere_kind::quad;
  opts.quad_resolution = 32;
  benchmark_sphere(state, opts);
}

BENCHMARK(BM_SphereUV_8x12);
BENCHMARK(BM_SphereUV_32x64);
BENCHMARK(BM_SphereIco_2);
BENCHMARK(BM_SphereIco_4);
BENCHMARK(BM_SphereIco_6);
BENCHMARK(BM_SphereQuad_8);
BENCHMARK(BM_SphereQuad_16);
BENCHMARK(BM_SphereQuad_32);

BENCHMARK_MAIN();
