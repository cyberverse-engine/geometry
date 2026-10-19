#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include <glm/geometric.hpp>

#include "cyberverse/geometry/bounds.hpp"
#include "cyberverse/geometry/primitives.hpp"
#include "test_helpers.hpp"

using namespace cyberverse::geometry;

namespace {

void check_mesh_invariants(const mesh_data& mesh) {
  REQUIRE(mesh.vertex_count() > 0);
  REQUIRE(test_support::indices_are_valid(mesh));
  REQUIRE(test_support::normals_are_unit(mesh));
  REQUIRE(test_support::has_no_degenerate_triangles(mesh));
}

void check_aabb(const mesh_data& mesh, const glm::vec3& min, const glm::vec3& max) {
  const aabb box = compute_aabb(mesh);
  REQUIRE(box.is_valid());
  REQUIRE(glm::length(box.min() - min) < 1e-5f);
  REQUIRE(glm::length(box.max() - max) < 1e-5f);
}

} // namespace

TEST_CASE("unit quad", "[primitives][quad]") {
  const mesh_data& mesh = unit_quad();
  REQUIRE(mesh.vertex_count() == 4);
  REQUIRE(mesh.triangle_count() == 2);
  REQUIRE(test_support::uvs_in_unit_range(mesh));
  check_mesh_invariants(mesh);

  for (const vertex& v : mesh.vertices()) {
    REQUIRE(std::abs(v.position.y) < 1e-6f);
    REQUIRE(glm::dot(v.normal, glm::vec3(0.0f, 1.0f, 0.0f)) > 0.999f);
  }

  for (size_t i = 0; i + 2 < mesh.indices().size(); i += 3) {
    const glm::vec3& v0 = mesh.vertices()[mesh.indices()[i]].position;
    const glm::vec3& v1 = mesh.vertices()[mesh.indices()[i + 1]].position;
    const glm::vec3& v2 = mesh.vertices()[mesh.indices()[i + 2]].position;
    REQUIRE(glm::dot(glm::cross(v1 - v0, v2 - v0), glm::vec3(0.0f, 1.0f, 0.0f)) > 0.0f);
  }
}

TEST_CASE("unit cube", "[primitives][cube]") {
  const mesh_data& mesh = unit_cube();
  REQUIRE(mesh.vertex_count() == 24);
  REQUIRE(mesh.triangle_count() == 12);
  check_mesh_invariants(mesh);
  check_aabb(mesh, { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f });
  REQUIRE(test_support::triangles_face_outward(mesh, glm::vec3(0.0f)));
}

TEST_CASE("box and slab", "[primitives][box]") {
  const mesh_data box = create_box(2.0f, 4.0f, 6.0f);
  REQUIRE(box.vertex_count() == 24);
  REQUIRE(box.triangle_count() == 12);
  check_mesh_invariants(box);
  check_aabb(box, { -1.0f, -2.0f, -3.0f }, { 1.0f, 2.0f, 3.0f });
  REQUIRE(test_support::triangles_face_outward(box, glm::vec3(0.0f)));

  const mesh_data slab = create_unit_slab(0.2f);
  REQUIRE(slab.vertex_count() == 24);
  REQUIRE(slab.triangle_count() == 12);
  check_aabb(slab, { -0.5f, -0.5f, -0.1f }, { 0.5f, 0.5f, 0.1f });

  REQUIRE(create_box(0.0f, 1.0f, 1.0f).empty());
  REQUIRE(create_box(1.0f, -1.0f, 1.0f).empty());
  REQUIRE(create_unit_slab(-0.1f).empty());
}

TEST_CASE("cylinder", "[primitives][cylinder]") {
  cylinder_options opts;
  opts.segments = 12;
  opts.caps = true;

  const mesh_data mesh = create_cylinder(2.0f, 4.0f, opts);
  REQUIRE(mesh.vertex_count() == 54);
  REQUIRE(mesh.triangle_count() == 48);
  check_mesh_invariants(mesh);
  check_aabb(mesh, { -2.0f, -2.0f, -2.0f }, { 2.0f, 2.0f, 2.0f });
  REQUIRE(test_support::triangles_face_outward(mesh, glm::vec3(0.0f)));

  opts.caps = false;
  const mesh_data open = create_cylinder(2.0f, 4.0f, opts);
  REQUIRE(open.vertex_count() == 26);
  REQUIRE(open.triangle_count() == 24);
  check_mesh_invariants(open);

  opts.caps = true;
  opts.segments = 1;
  const mesh_data clamped = create_cylinder(2.0f, 4.0f, opts);
  REQUIRE(clamped.vertex_count() == 18);
  REQUIRE(clamped.triangle_count() == 12);

  opts.segments = 12;
  opts.normal = normal_mode::flat;
  const mesh_data flat = create_cylinder(2.0f, 4.0f, opts);
  REQUIRE(flat.vertex_count() == flat.triangle_count() * 3);
  REQUIRE(test_support::flat_normals_consistent(flat));

  opts.normal = normal_mode::smooth;
  REQUIRE(create_cylinder(0.0f, 4.0f, opts).empty());
  REQUIRE(create_cylinder(1.0f, 0.0f, opts).empty());
}

TEST_CASE("cone", "[primitives][cone]") {
  const mesh_data mesh = create_cone(2.0f, 4.0f, 12);
  REQUIRE(mesh.vertex_count() == 39);
  REQUIRE(mesh.triangle_count() == 24);
  check_mesh_invariants(mesh);
  check_aabb(mesh, { -2.0f, 0.0f, -2.0f }, { 2.0f, 4.0f, 2.0f });
  REQUIRE(test_support::triangles_face_outward(mesh, glm::vec3(0.0f, 2.0f, 0.0f)));

  const mesh_data clamped = create_cone(2.0f, 4.0f, 1);
  REQUIRE(clamped.vertex_count() == 12);
  REQUIRE(clamped.triangle_count() == 6);

  REQUIRE(create_cone(0.0f, 4.0f, 12).empty());
  REQUIRE(create_cone(1.0f, -1.0f, 12).empty());
}
