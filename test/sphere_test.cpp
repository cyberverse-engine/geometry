#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <limits>

#include "cyberverse/geometry/bounds.hpp"
#include "cyberverse/geometry/primitives.hpp"
#include "test_helpers.hpp"

using namespace cyberverse::geometry;

namespace {

constexpr float k_radius = 2.3f;
constexpr float k_pi = 3.14159265358979323846f;

sphere_options uv_options(int rings, int segments) {
  sphere_options opts;
  opts.kind = sphere_kind::uv;
  opts.rings = rings;
  opts.segments = segments;
  return opts;
}

sphere_options ico_options(int subdivisions) {
  sphere_options opts;
  opts.kind = sphere_kind::ico;
  opts.ico_subdivisions = subdivisions;
  return opts;
}

sphere_options quad_options(int resolution) {
  sphere_options opts;
  opts.kind = sphere_kind::quad;
  opts.quad_resolution = resolution;
  return opts;
}

void check_sphere_invariants(const mesh_data& mesh) {
  REQUIRE(mesh.vertex_count() > 0);
  REQUIRE(test_support::indices_are_valid(mesh));
  REQUIRE(test_support::positions_on_sphere(mesh, k_radius));
  REQUIRE(test_support::normals_are_unit(mesh));
  REQUIRE(test_support::normals_point_outward(mesh));
  REQUIRE(test_support::has_no_degenerate_triangles(mesh, k_radius));
  REQUIRE(test_support::triangles_face_outward(mesh, glm::vec3(0.0f)));
  REQUIRE(test_support::unused_vertex_count(mesh) == 0);
}

void check_closed_sphere(const mesh_data& mesh, size_t welded_vertices, size_t edges) {
  const test_support::topology_summary topology =
      test_support::analyze_topology(mesh, test_support::relative_epsilon(k_radius));
  REQUIRE(topology.valid);
  REQUIRE(topology.vertex_count == welded_vertices);
  REQUIRE(topology.edge_count == edges);
  REQUIRE(topology.boundary_edges == 0);
  REQUIRE(topology.non_manifold_edges == 0);
  REQUIRE(topology.euler_characteristic == 2);
}

} // namespace

TEST_CASE("UV sphere counts, winding and topology", "[sphere][uv]") {
  const sphere_options opts = uv_options(8, 12);
  const mesh_data mesh = create_sphere(k_radius, opts);

  REQUIRE(mesh.vertex_count() == 93);
  REQUIRE(mesh.triangle_count() == 168);
  REQUIRE(test_support::uvs_in_unit_range(mesh));
  check_sphere_invariants(mesh);
  check_closed_sphere(mesh, 86, 252);
}

TEST_CASE("Icosphere counts, winding and topology", "[sphere][ico]") {
  const sphere_options opts = ico_options(3);
  const mesh_data mesh = create_sphere(k_radius, opts);

  REQUIRE(mesh.triangle_count() == 1280);
  REQUIRE(mesh.vertex_count() >= 642);
  REQUIRE(test_support::ico_uv_spans_ok(mesh));
  check_sphere_invariants(mesh);
  check_closed_sphere(mesh, 642, 1920);
}

TEST_CASE("Quad sphere counts, winding and topology", "[sphere][quad]") {
  const sphere_options opts = quad_options(8);
  const mesh_data mesh = create_sphere(k_radius, opts);

  REQUIRE(mesh.vertex_count() == 486);
  REQUIRE(mesh.triangle_count() == 768);
  REQUIRE(test_support::uvs_in_unit_range(mesh));
  check_sphere_invariants(mesh);
  check_closed_sphere(mesh, 386, 1152);
}

TEST_CASE("Sphere flat normals produce one vertex per index", "[sphere][flat]") {
  sphere_options uv_opts = uv_options(4, 6);
  uv_opts.normal = normal_mode::flat;
  sphere_options ico_opts = ico_options(1);
  ico_opts.normal = normal_mode::flat;
  sphere_options quad_opts = quad_options(2);
  quad_opts.normal = normal_mode::flat;

  const mesh_data uv = create_sphere(k_radius, uv_opts);
  const mesh_data ico = create_sphere(k_radius, ico_opts);
  const mesh_data quad = create_sphere(k_radius, quad_opts);

  for (const mesh_data* mesh : { &uv, &ico, &quad }) {
    REQUIRE(mesh->vertex_count() == mesh->triangle_count() * 3);
    REQUIRE(test_support::indices_are_valid(*mesh));
    REQUIRE(test_support::positions_on_sphere(*mesh, k_radius));
    REQUIRE(test_support::flat_normals_consistent(*mesh));
    REQUIRE(test_support::triangles_face_outward(*mesh, glm::vec3(0.0f)));
  }
}

TEST_CASE("Sphere surface area converges to 4*pi*r^2", "[sphere][area]") {
  const float expected = 4.0f * k_pi * k_radius * k_radius;
  const mesh_data uv = create_sphere(k_radius, uv_options(32, 64));
  const mesh_data ico = create_sphere(k_radius, ico_options(4));
  const mesh_data quad = create_sphere(k_radius, quad_options(32));

  for (const mesh_data* mesh : { &uv, &ico, &quad }) {
    const float relative_error = std::abs(test_support::surface_area(*mesh) - expected) / expected;
    REQUIRE(relative_error < 0.01f);
  }
}

TEST_CASE("Sphere is centered and stays inside its radius", "[sphere][bounds]") {
  for (const sphere_options& opts : { uv_options(8, 12), ico_options(3), quad_options(8) }) {
    const mesh_data mesh = create_sphere(k_radius, opts);
    const aabb box = compute_aabb(mesh);

    REQUIRE(box.is_valid());
    REQUIRE(glm::length(box.center()) < 1e-4f);
    for (int axis = 0; axis < 3; ++axis) {
      REQUIRE(box.min()[axis] >= -k_radius - 1e-4f);
      REQUIRE(box.max()[axis] <= k_radius + 1e-4f);
    }
  }
}

TEST_CASE("Sphere parameter clamps", "[sphere][clamp]") {
  const mesh_data uv = create_sphere(1.0f, uv_options(0, 1));
  REQUIRE(uv.vertex_count() == 6);
  REQUIRE(uv.triangle_count() == 6);

  const mesh_data ico = create_sphere(1.0f, ico_options(-5));
  REQUIRE(ico.triangle_count() == 20);
  REQUIRE(test_support::analyze_topology(ico, 1e-4f).vertex_count == 12);

  const mesh_data quad = create_sphere(1.0f, quad_options(0));
  REQUIRE(quad.vertex_count() == 24);
  REQUIRE(quad.triangle_count() == 12);
}

TEST_CASE("Sphere rejects invalid radii", "[sphere][invalid]") {
  REQUIRE(create_sphere(0.0f, uv_options(8, 12)).empty());
  REQUIRE(create_sphere(-1.0f, uv_options(8, 12)).empty());
  REQUIRE(create_sphere(std::nanf(""), uv_options(8, 12)).empty());
  REQUIRE(create_sphere(std::numeric_limits<float>::infinity(), uv_options(8, 12)).empty());
}

TEST_CASE("Sphere options keep the legacy aggregate initialization", "[sphere][compat]") {
  const sphere_options legacy{ 6, 8, normal_mode::smooth };
  REQUIRE(legacy.kind == sphere_kind::uv);

  const mesh_data mesh = create_sphere(1.0f, legacy);
  REQUIRE(mesh.triangle_count() == 80);

  const mesh_data defaults = create_sphere(1.0f);
  REQUIRE(defaults.vertex_count() == 93);
  REQUIRE(defaults.triangle_count() == 168);
}

TEST_CASE("Sphere generation is deterministic", "[sphere][deterministic]") {
  for (const sphere_options& opts : { uv_options(8, 12), ico_options(3), quad_options(8) }) {
    const mesh_data first = create_sphere(k_radius, opts);
    const mesh_data second = create_sphere(k_radius, opts);
    REQUIRE(test_support::meshes_equal(first, second));
  }
}
