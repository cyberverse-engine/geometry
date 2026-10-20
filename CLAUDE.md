# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository

`geometry` is the standalone geometry module of Cyberverse Engine: C++23, GLM, built as the static library `cyberverse::geometry::static_lib`. The public API lives in `src/cyberverse/geometry/`; `src/cyberverse/geometry.hpp` is the umbrella header. All code is hand-written headers and `.cpp` files.

## Build and test

Run from this directory. Tests must be built and run under `/tmp`, never inside the repo.

```bash
cmake -S . -B build
cmake --build build

cmake -S . -B /tmp/cyberverse-geometry-test
cmake --build /tmp/cyberverse-geometry-test
ctest --test-dir /tmp/cyberverse-geometry-test
```

- GLM 1.0.1 is fetched with `FetchContent` in `src/CMakeLists.txt` and linked as `glm::glm`; add dependencies the same way.
- Default configure also fetches Catch2 v3.5.2 and Google Benchmark v1.9.5. Skip them with `-DCYBERVERSE_GEOMETRY_BUILD_TESTS=OFF -DCYBERVERSE_GEOMETRY_BUILD_BENCHMARKS=OFF`.
- Presets: configure once with `cmake --preset debug`, then build with `cmake --build --preset debug` (same for `release`); build dirs are `build/` and `build-release/`.
- `test/` builds `cyberverse_geometry_tests` (Catch2, discovered by CTest); `benchmark/` builds `cyberverse_geometry_benchmarks` (Google Benchmark). Tests must still be built and run under `/tmp`.
- `docs/roadmap.md` records the capability audit and prioritized backlog.

## Geometry conventions

- Right-handed, Y-up. `unit_quad` lies in XZ at y=0 with a +Y normal; `unit_cube` is centered at the origin; `create_unit_slab` is 1x1xthickness along Z; `create_cylinder` is centered on +Y; `create_cone` has its base center at the origin and points +Y; `create_sphere` is centered at the origin.
- `create_sphere` selects the algorithm through `sphere_options::kind`: `uv` (lat/long, default), `ico` (subdivided icosahedron), or `quad` (spherified cube). Smooth normals point along the radius; `normal_mode::flat` duplicates vertices per triangle. Invalid radii return an empty mesh. Keep the triangle counts and UV conventions documented in `primitives.hpp`.
- Triangles are counter-clockwise when viewed from outside and vertex normals agree with that winding. Keep both invariants when adding primitives.
- `mesh_data` stores vertices and indices separately; every three indices form one triangle and `triangle_count()` is `index_count() / 3`. `append()` offsets the appended indices. `empty()` is based on vertices, so a vertex-only mesh has no drawable triangles.
- Rays use `p = origin + t * direction`, where `direction` is unit length and `t` is distance. The `ray` constructor normalizes non-zero directions; zero or non-finite directions produce an invalid ray and every intersection returns false. Use `ray::from_normalized` when the direction is already unit length. `ray_mesh` returns the nearest distance and its triangle index.
- `frustum` stores six inward-facing planes (`dot(normal, p) - d >= 0` is inside); plane normals must be unit length. `is_outside` is exact for points and a conservative plane test for AABBs/bounding spheres. Invalid AABBs and spheres, plus non-finite points, are treated as outside.
- `aabb` is invalid by default and grows through `expand()`; `compute_aabb` of an empty mesh stays invalid. `compute_bounding_sphere` uses the AABB center and the farthest vertex, and returns a zero-radius sphere for an empty mesh.
- `aabb_overlaps` and `sphere_overlaps_aabb` use closed intervals (touching counts) and return false for invalid or non-finite inputs. `point_aabb_distance_sq` returns 0 inside the box and `+infinity` for an invalid box or non-finite point.

## Style

- `.clang-format` is authoritative: LLVM base, 2-space indent, 120 columns, attached braces, left-aligned pointers.
- Public types and functions are `lower_snake_case`; private members end with `_`.
- Use `#pragma once`, include public headers as `"cyberverse/geometry/<name>.hpp"`, and keep the Doxygen `@file` header.
- Add new `.cpp` files to `SOURCE_FILES` and public headers to `HEADER_FILES` in `src/CMakeLists.txt`, and include new public headers from `geometry.hpp`.

`AGENTS.md` is a symlink to this file; edit only `CLAUDE.md`.
