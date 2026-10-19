/**
 * @file bounds.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Bounds implementation
 * @version 0.1
 * @date 2026-10-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/geometry/bounds.hpp"

#include <algorithm>
#include <cmath>

#include <glm/geometric.hpp>

namespace cyberverse {
namespace geometry {

aabb::aabb(const glm::vec3& min, const glm::vec3& max) noexcept : min_(min), max_(max) {}

bool aabb::is_valid() const noexcept { return min_.x <= max_.x && min_.y <= max_.y && min_.z <= max_.z; }

const glm::vec3& aabb::min() const noexcept { return min_; }

const glm::vec3& aabb::max() const noexcept { return max_; }

glm::vec3 aabb::center() const noexcept { return (min_ + max_) * 0.5f; }

void aabb::expand(const glm::vec3& point) noexcept {
  min_ = glm::min(min_, point);
  max_ = glm::max(max_, point);
}

void aabb::expand(const aabb& other) noexcept {
  if (!other.is_valid()) {
    return;
  }

  expand(other.min_);
  expand(other.max_);
}

bounding_sphere::bounding_sphere(const glm::vec3& center, float radius) noexcept : center_(center), radius_(radius) {}

const glm::vec3& bounding_sphere::center() const noexcept { return center_; }

float bounding_sphere::radius() const noexcept { return radius_; }

bool bounding_sphere::is_valid() const noexcept { return radius_ >= 0.0f; }

aabb compute_aabb(const mesh_data& mesh) {
  aabb result;
  for (const vertex& v : mesh.vertices()) {
    result.expand(v.position);
  }
  return result;
}

bounding_sphere compute_bounding_sphere(const mesh_data& mesh) {
  const aabb box = compute_aabb(mesh);
  if (!box.is_valid()) {
    return {};
  }

  const glm::vec3 center = box.center();
  float radius_squared = 0.0f;
  for (const vertex& v : mesh.vertices()) {
    const glm::vec3 delta = v.position - center;
    radius_squared = std::max(radius_squared, glm::dot(delta, delta));
  }

  return bounding_sphere(center, std::sqrt(radius_squared));
}

} // namespace geometry
} // namespace cyberverse
