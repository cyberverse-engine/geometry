# geometry

Cyberverse Engine geometry library: mesh data, bounds, ray intersections, and primitive
generation (C++23, GLM).

## Features

- `mesh_data` with position, normal, and UV vertices plus 32-bit triangle indices.
- AABB and bounding-sphere computation.
- Ray intersections against planes, AABBs, bounding spheres, triangles, and triangle meshes.
  `ray` stores a unit-length direction, so `t` is the hit distance; construction normalizes
  any non-zero direction.
- Frustum culling for points, AABBs, and bounding spheres, using inward-facing planes.
- Primitives: unit quad, unit cube, box, slab, cylinder, cone.
- Spheres: UV sphere, icosphere, and quad sphere through `create_sphere(radius, sphere_options)`.
- Catch2 tests and Google Benchmark benchmarks.

## Build

```bash
cmake -S . -B build
cmake --build build
```

Tests must be configured, built, and run under `/tmp`:

```bash
cmake -S . -B /tmp/cyberverse-geometry-test
cmake --build /tmp/cyberverse-geometry-test
ctest --test-dir /tmp/cyberverse-geometry-test --output-on-failure
```

Disable optional targets with
`-DCYBERVERSE_GEOMETRY_BUILD_TESTS=OFF -DCYBERVERSE_GEOMETRY_BUILD_BENCHMARKS=OFF`.

## Roadmap

See [docs/roadmap.md](docs/roadmap.md) for the capability audit and prioritized backlog.
