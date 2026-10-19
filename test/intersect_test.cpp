#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <limits>
#include <utility>

#include <glm/geometric.hpp>

#include "cyberverse/geometry/intersect.hpp"
#include "cyberverse/geometry/primitives.hpp"

using namespace cyberverse::geometry;

namespace {

constexpr float k_epsilon = 1e-5f;

bool approx(float lhs, float rhs) { return std::abs(lhs - rhs) < k_epsilon; }

bool same_vec(const glm::vec3& lhs, const glm::vec3& rhs) { return glm::length(lhs - rhs) < k_epsilon; }

mesh_data make_two_triangles() {
  std::vector<vertex> vertices = {
    { { -1.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    { { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    { { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
  };
  return mesh_data(std::move(vertices), { 0, 1, 2, 3, 4, 5 });
}

} // namespace

TEST_CASE("ray construction and validity", "[intersect][ray]") {
  const ray normalized({ 2.0f, 3.0f, 4.0f }, { 0.0f, 2.0f, 0.0f });
  REQUIRE(normalized.is_valid());
  REQUIRE(same_vec(normalized.origin(), { 2.0f, 3.0f, 4.0f }));
  REQUIRE(same_vec(normalized.direction(), { 0.0f, 1.0f, 0.0f }));

  const ray tiny({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1e-8f, 0.0f });
  REQUIRE(tiny.is_valid());
  REQUIRE(same_vec(tiny.direction(), { 0.0f, 1.0f, 0.0f }));

  const ray from_unit = ray::from_normalized({ 1.0f, 2.0f, 3.0f }, { 0.0f, 0.0f, 1.0f });
  REQUIRE(from_unit.is_valid());
  REQUIRE(same_vec(from_unit.direction(), { 0.0f, 0.0f, 1.0f }));

  REQUIRE_FALSE(ray{}.is_valid());
  REQUIRE_FALSE(ray({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }).is_valid());
  REQUIRE_FALSE(ray({ std::nanf(""), 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }).is_valid());
  REQUIRE_FALSE(ray({ 0.0f, 0.0f, 0.0f }, { std::nanf(""), 0.0f, 0.0f }).is_valid());
  REQUIRE_FALSE(ray({ 0.0f, 0.0f, 0.0f }, { std::numeric_limits<float>::infinity(), 0.0f, 0.0f }).is_valid());
}

TEST_CASE("invalid ray misses every intersection", "[intersect][ray]") {
  const ray invalid;
  const plane plane_xz{ { 0.0f, 1.0f, 0.0f }, 0.0f };
  const aabb box({ -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f });
  const bounding_sphere sphere({ 0.0f, 0.0f, 0.0f }, 1.0f);
  const mesh_data& quad = unit_quad();
  float t = 0.0f;
  uint32_t triangle = 0;

  REQUIRE_FALSE(ray_plane(invalid, plane_xz, t));
  REQUIRE_FALSE(ray_aabb(invalid, box, t));
  REQUIRE_FALSE(ray_bounding_sphere(invalid, sphere, t));
  REQUIRE_FALSE(ray_triangle(invalid, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, t));
  REQUIRE_FALSE(ray_mesh(invalid, quad, t, triangle));
}

TEST_CASE("ray_plane", "[intersect][ray]") {
  const plane horizontal{ { 0.0f, 1.0f, 0.0f }, 1.0f };
  float t = 0.0f;

  REQUIRE(ray_plane(ray({ 0.0f, 0.0f, 0.0f }, { 0.0f, 2.0f, 0.0f }), horizontal, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_plane(ray({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.5f, 0.0f }), horizontal, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_plane(ray({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1e-8f, 0.0f }), horizontal, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_plane(ray({ 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }), horizontal, t));
  REQUIRE(approx(t, 0.0f));

  REQUIRE_FALSE(ray_plane(ray({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), horizontal, t));
  REQUIRE_FALSE(ray_plane(ray({ 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }), horizontal, t));
}

TEST_CASE("ray_aabb", "[intersect][ray]") {
  const aabb box({ -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f });
  float t = 0.0f;

  REQUIRE(ray_aabb(ray({ -2.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), box, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_aabb(ray({ -2.0f, 0.0f, 0.0f }, { 2.0f, 0.0f, 0.0f }), box, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_aabb(ray({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), box, t));
  REQUIRE(approx(t, 0.0f));

  REQUIRE_FALSE(ray_aabb(ray({ 0.0f, 2.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), box, t));
  REQUIRE_FALSE(ray_aabb(ray({ 0.0f, 2.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }), box, t));
  REQUIRE_FALSE(ray_aabb(ray({ -2.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), aabb{}, t));
}

TEST_CASE("ray_bounding_sphere", "[intersect][ray]") {
  const bounding_sphere sphere({ 0.0f, 0.0f, 0.0f }, 1.0f);
  float t = 0.0f;

  REQUIRE(ray_bounding_sphere(ray({ -2.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), sphere, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_bounding_sphere(ray({ -2.0f, 0.0f, 0.0f }, { 2.0f, 0.0f, 0.0f }), sphere, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_bounding_sphere(ray({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), sphere, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE_FALSE(ray_bounding_sphere(ray({ 0.0f, 2.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), sphere, t));
  REQUIRE_FALSE(ray_bounding_sphere(ray({ -2.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }), sphere, t));
  REQUIRE_FALSE(ray_bounding_sphere(
      ray({ -2.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), bounding_sphere({ 0.0f, 0.0f, 0.0f }, -1.0f), t));
}

TEST_CASE("ray_triangle", "[intersect][ray]") {
  const glm::vec3 v0(0.0f, 0.0f, 0.0f);
  const glm::vec3 v1(1.0f, 0.0f, 0.0f);
  const glm::vec3 v2(0.0f, 1.0f, 0.0f);
  float t = 0.0f;

  REQUIRE(ray_triangle(ray({ 0.25f, 0.25f, 1.0f }, { 0.0f, 0.0f, -1.0f }), v0, v1, v2, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_triangle(ray({ 0.25f, 0.25f, 1.0f }, { 0.0f, 0.0f, -2.0f }), v0, v1, v2, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_triangle(ray({ 0.25f, 0.25f, -1.0f }, { 0.0f, 0.0f, 1.0f }), v0, v1, v2, t));
  REQUIRE(approx(t, 1.0f));

  REQUIRE_FALSE(ray_triangle(ray({ 1.5f, 0.25f, 1.0f }, { 0.0f, 0.0f, -1.0f }), v0, v1, v2, t));
  REQUIRE_FALSE(ray_triangle(ray({ 0.25f, 0.25f, 1.0f }, { 1.0f, 0.0f, 0.0f }), v0, v1, v2, t));
  REQUIRE_FALSE(ray_triangle(ray({ 0.25f, 0.25f, 1.0f }, { 0.0f, 0.0f, 1.0f }), v0, v1, v2, t));
}

TEST_CASE("ray_mesh", "[intersect][ray]") {
  float t = 0.0f;
  uint32_t triangle = 0;

  const mesh_data& quad = unit_quad();
  REQUIRE(ray_mesh(ray({ 0.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }), quad, t, triangle));
  REQUIRE(approx(t, 1.0f));
  REQUIRE(triangle < quad.triangle_count());

  REQUIRE(ray_mesh(ray({ 0.0f, 1.0f, 0.0f }, { 0.0f, -2.0f, 0.0f }), quad, t, triangle));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_mesh(ray({ 0.0f, 1.0f, 0.0f }, { 0.0f, -1e-8f, 0.0f }), quad, t, triangle));
  REQUIRE(approx(t, 1.0f));

  REQUIRE(ray_mesh(ray({ 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }), quad, t, triangle));
  REQUIRE(approx(t, 1.0f));

  REQUIRE_FALSE(ray_mesh(ray({ 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }), quad, t, triangle));
  REQUIRE_FALSE(ray_mesh(ray({ 0.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }), mesh_data{}, t, triangle));

  const mesh_data two_triangles = make_two_triangles();
  REQUIRE(ray_mesh(ray({ 0.0f, 2.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }), two_triangles, t, triangle));
  REQUIRE(approx(t, 1.0f));
  REQUIRE(triangle == 1);
}
