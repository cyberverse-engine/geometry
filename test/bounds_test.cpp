#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include <glm/geometric.hpp>

#include "cyberverse/geometry/bounds.hpp"

using namespace cyberverse::geometry;

namespace {

bool same_vec(const glm::vec3& lhs, const glm::vec3& rhs) { return glm::length(lhs - rhs) < 1e-6f; }

} // namespace

TEST_CASE("aabb default and expansion", "[bounds][aabb]") {
  aabb box;
  REQUIRE_FALSE(box.is_valid());

  box.expand({ 1.0f, 2.0f, 3.0f });
  REQUIRE(box.is_valid());
  REQUIRE(same_vec(box.min(), { 1.0f, 2.0f, 3.0f }));
  REQUIRE(same_vec(box.max(), { 1.0f, 2.0f, 3.0f }));
  REQUIRE(same_vec(box.center(), { 1.0f, 2.0f, 3.0f }));

  box.expand({ -1.0f, 5.0f, 0.0f });
  REQUIRE(same_vec(box.min(), { -1.0f, 2.0f, 0.0f }));
  REQUIRE(same_vec(box.max(), { 1.0f, 5.0f, 3.0f }));
  REQUIRE(same_vec(box.center(), { 0.0f, 3.5f, 1.5f }));
}

TEST_CASE("aabb explicit bounds and box expansion", "[bounds][aabb]") {
  const aabb box({ -1.0f, -2.0f, -3.0f }, { 1.0f, 2.0f, 3.0f });
  REQUIRE(box.is_valid());

  aabb other;
  other.expand(box);
  REQUIRE(same_vec(other.min(), box.min()));
  REQUIRE(same_vec(other.max(), box.max()));

  aabb expanded;
  expanded.expand(box);
  expanded.expand(aabb({ 2.0f, 2.0f, 2.0f }, { 4.0f, 4.0f, 4.0f }));
  REQUIRE(same_vec(expanded.min(), { -1.0f, -2.0f, -3.0f }));
  REQUIRE(same_vec(expanded.max(), { 4.0f, 4.0f, 4.0f }));

  aabb invalid_box;
  expanded.expand(invalid_box);
  REQUIRE(same_vec(expanded.min(), { -1.0f, -2.0f, -3.0f }));
  REQUIRE(same_vec(expanded.max(), { 4.0f, 4.0f, 4.0f }));

  REQUIRE_FALSE(aabb({ 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }).is_valid());
}

TEST_CASE("compute_aabb", "[bounds][aabb]") {
  REQUIRE_FALSE(compute_aabb(mesh_data{}).is_valid());

  mesh_data mesh;
  mesh.vertices().push_back({ { 1.0f, 2.0f, 3.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
  mesh.vertices().push_back({ { -2.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
  mesh.vertices().push_back({ { 0.0f, 4.0f, -3.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });

  const aabb box = compute_aabb(mesh);
  REQUIRE(box.is_valid());
  REQUIRE(same_vec(box.min(), { -2.0f, -1.0f, -3.0f }));
  REQUIRE(same_vec(box.max(), { 1.0f, 4.0f, 3.0f }));
}

TEST_CASE("compute_bounding_sphere", "[bounds][sphere]") {
  const bounding_sphere empty = compute_bounding_sphere(mesh_data{});
  REQUIRE(empty.is_valid());
  REQUIRE(empty.radius() == 0.0f);
  REQUIRE(glm::length(empty.center()) < 1e-6f);

  mesh_data line;
  line.vertices().push_back({ { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
  line.vertices().push_back({ { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
  const bounding_sphere line_sphere = compute_bounding_sphere(line);
  REQUIRE(glm::length(line_sphere.center()) < 1e-6f);
  REQUIRE(std::abs(line_sphere.radius() - 1.0f) < 1e-6f);

  mesh_data point;
  point.vertices().push_back({ { 2.0f, 3.0f, 4.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
  const bounding_sphere point_sphere = compute_bounding_sphere(point);
  REQUIRE(same_vec(point_sphere.center(), { 2.0f, 3.0f, 4.0f }));
  REQUIRE(point_sphere.radius() == 0.0f);

  REQUIRE_FALSE(bounding_sphere({ 0.0f, 0.0f, 0.0f }, -1.0f).is_valid());
}
