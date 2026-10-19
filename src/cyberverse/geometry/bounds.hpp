/**
 * @file bounds.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Bound data structures and algorithms
 * @version 0.1
 * @date 2026-10-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <limits>

#include "cyberverse/geometry/mesh_data.hpp"

namespace cyberverse {
namespace geometry {

/**
 * @brief Axis-aligned bounding box
 *
 * A default-constructed box is invalid (min > max) and can be grown with
 * expand(); is_valid() reports whether it contains a non-empty volume.
 */
class aabb {
public:
  aabb() = default;
  aabb(const glm::vec3& min, const glm::vec3& max) noexcept;

  /**
   * @brief Is a validate aabb box
   *
   * @return true
   * @return false
   */
  bool is_valid() const noexcept;

  const glm::vec3& min() const noexcept;
  const glm::vec3& max() const noexcept;
  glm::vec3 center() const noexcept;

  /**
   * @brief Grow the box to include a point
   *
   * @param point
   */
  void expand(const glm::vec3& point) noexcept;

  /**
   * @brief Grow the box to include another box
   *
   * @param other
   */
  void expand(const aabb& other) noexcept;

private:
  glm::vec3 min_{ std::numeric_limits<float>::max() };
  glm::vec3 max_{ std::numeric_limits<float>::lowest() };
};

/**
 * @brief Bounding sphere
 *
 */
class bounding_sphere {
public:
  bounding_sphere() noexcept = default;
  explicit bounding_sphere(const glm::vec3& center, float radius = 0.0f) noexcept;

  const glm::vec3& center() const noexcept;
  float radius() const noexcept;

  /**
   * @brief Is the radius non-negative
   *
   * @return true
   * @return false
   */
  bool is_valid() const noexcept;

private:
  glm::vec3 center_{ 0.0f };
  float radius_ = 0.0f;
};

/**
 * @brief Compute aabb
 *
 * @param mesh Mesh data
 * @return aabb Invalid when the mesh has no vertices
 */
aabb compute_aabb(const mesh_data& mesh);

/**
 * @brief Compute bounding sphere
 *
 * Uses the center of the aabb and the farthest vertex. Returns a zero-radius
 * sphere at the origin for an empty mesh.
 *
 * @param mesh Mesh data
 * @return bounding_sphere
 */
bounding_sphere compute_bounding_sphere(const mesh_data& mesh);

} // namespace geometry
} // namespace cyberverse
