#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "cyberverse/geometry/intersect.hpp"

using namespace cyberverse::geometry;

namespace {

// Frustum equivalent to the axis-aligned box [-1, 1]^3, with inward normals.
frustum make_box_frustum() {
  frustum f;
  f.planes[0] = { { 1.0f, 0.0f, 0.0f }, -1.0f };  // x >= -1
  f.planes[1] = { { -1.0f, 0.0f, 0.0f }, -1.0f }; // x <= 1
  f.planes[2] = { { 0.0f, 1.0f, 0.0f }, -1.0f };  // y >= -1
  f.planes[3] = { { 0.0f, -1.0f, 0.0f }, -1.0f }; // y <= 1
  f.planes[4] = { { 0.0f, 0.0f, 1.0f }, -1.0f };  // z >= -1
  f.planes[5] = { { 0.0f, 0.0f, -1.0f }, -1.0f }; // z <= 1
  return f;
}

} // namespace

TEST_CASE("frustum point classification", "[frustum][is_outside]") {
  const frustum f = make_box_frustum();

  REQUIRE_FALSE(is_outside(f, glm::vec3(0.0f, 0.0f, 0.0f)));
  REQUIRE_FALSE(is_outside(f, glm::vec3(1.0f, 1.0f, 1.0f)));
  REQUIRE_FALSE(is_outside(f, glm::vec3(-1.0f, 0.0f, 0.5f)));

  REQUIRE(is_outside(f, glm::vec3(1.01f, 0.0f, 0.0f)));
  REQUIRE(is_outside(f, glm::vec3(-2.0f, 0.0f, 0.0f)));
  REQUIRE(is_outside(f, glm::vec3(0.0f, 0.0f, 1.5f)));
  REQUIRE(is_outside(f, glm::vec3(0.0f, std::nanf(""), 0.0f)));
}

TEST_CASE("frustum AABB culling", "[frustum][is_outside]") {
  const frustum f = make_box_frustum();

  REQUIRE_FALSE(is_outside(f, aabb({ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f })));
  REQUIRE_FALSE(is_outside(f, aabb({ -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f })));
  REQUIRE_FALSE(is_outside(f, aabb({ -2.0f, -2.0f, -2.0f }, { 2.0f, 2.0f, 2.0f })));

  // Touching the x = 1 face is not outside.
  REQUIRE_FALSE(is_outside(f, aabb({ 1.0f, 0.0f, 0.0f }, { 2.0f, 1.0f, 1.0f })));

  REQUIRE(is_outside(f, aabb({ 1.1f, 0.0f, 0.0f }, { 2.0f, 1.0f, 1.0f })));
  REQUIRE(is_outside(f, aabb({ -3.0f, -3.0f, -3.0f }, { -2.0f, -2.0f, -2.0f })));
  REQUIRE(is_outside(f, aabb{}));
}

TEST_CASE("frustum bounding sphere culling", "[frustum][is_outside]") {
  const frustum f = make_box_frustum();

  REQUIRE_FALSE(is_outside(f, bounding_sphere({ 0.0f, 0.0f, 0.0f }, 0.5f)));
  REQUIRE_FALSE(is_outside(f, bounding_sphere({ 0.0f, 0.0f, 0.0f }, 2.0f)));

  // Tangent at x = 1 is not outside.
  REQUIRE_FALSE(is_outside(f, bounding_sphere({ 1.5f, 0.0f, 0.0f }, 0.5f)));

  REQUIRE(is_outside(f, bounding_sphere({ 1.6f, 0.0f, 0.0f }, 0.5f)));
  REQUIRE(is_outside(f, bounding_sphere({ -2.0f, 0.0f, 0.0f }, 0.5f)));
  REQUIRE(is_outside(f, bounding_sphere({ 0.0f, 0.0f, 0.0f }, -1.0f)));
  REQUIRE(is_outside(f, bounding_sphere({ std::nanf(""), 0.0f, 0.0f }, 1.0f)));
}

TEST_CASE("default frustum culls nothing", "[frustum][is_outside]") {
  const frustum f;
  REQUIRE_FALSE(is_outside(f, glm::vec3(0.0f, 0.0f, 0.0f)));
  REQUIRE_FALSE(is_outside(f, glm::vec3(100.0f, -100.0f, 50.0f)));
}
