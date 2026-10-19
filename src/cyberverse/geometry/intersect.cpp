/**
 * @file intersect.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Intersect implementation
 * @version 0.1
 * @date 2026-10-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/geometry/intersect.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

#include <glm/geometric.hpp>

namespace cyberverse {
namespace geometry {
namespace {

constexpr float k_epsilon = 1e-7f;
constexpr float k_unit_epsilon = 1e-3f;

bool is_finite(const glm::vec3& value) noexcept {
  return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool ray_triangle_impl(const glm::vec3& origin,
                       const glm::vec3& direction,
                       const glm::vec3& v0,
                       const glm::vec3& v1,
                       const glm::vec3& v2,
                       float& out_t) {
  const glm::vec3 edge1 = v1 - v0;
  const glm::vec3 edge2 = v2 - v0;
  const glm::vec3 pvec = glm::cross(direction, edge2);
  const float determinant = glm::dot(edge1, pvec);
  if (std::abs(determinant) < k_epsilon) {
    return false;
  }

  const float inverse = 1.0f / determinant;
  const glm::vec3 tvec = origin - v0;
  const float u = glm::dot(tvec, pvec) * inverse;
  if (u < 0.0f || u > 1.0f) {
    return false;
  }

  const glm::vec3 qvec = glm::cross(tvec, edge1);
  const float v = glm::dot(direction, qvec) * inverse;
  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }

  const float t = glm::dot(edge2, qvec) * inverse;
  if (t < 0.0f) {
    return false;
  }

  out_t = t;
  return true;
}

} // namespace

ray::ray(const glm::vec3& origin, const glm::vec3& direction) noexcept : origin_(origin) {
  if (!is_finite(origin) || !is_finite(direction)) {
    return;
  }

  const float length_squared = glm::dot(direction, direction);
  if (!std::isfinite(length_squared) || !(length_squared > 0.0f)) {
    return;
  }

  direction_ = direction / std::sqrt(length_squared);
}

ray ray::from_normalized(const glm::vec3& origin, const glm::vec3& unit_direction) noexcept {
  assert(is_finite(origin) && is_finite(unit_direction));
  assert(std::abs(glm::dot(unit_direction, unit_direction) - 1.0f) <= k_unit_epsilon);

  ray result;
  result.origin_ = origin;
  result.direction_ = unit_direction;
  return result;
}

const glm::vec3& ray::origin() const noexcept { return origin_; }

const glm::vec3& ray::direction() const noexcept { return direction_; }

bool ray::is_valid() const noexcept {
  if (!is_finite(origin_) || !is_finite(direction_)) {
    return false;
  }
  return std::abs(glm::dot(direction_, direction_) - 1.0f) <= k_unit_epsilon;
}

bool ray_plane(const ray& ry, const plane& pn, float& out_t) {
  if (!ry.is_valid()) {
    return false;
  }

  const glm::vec3& origin = ry.origin();
  const glm::vec3& direction = ry.direction();
  const float denominator = glm::dot(pn.normal, direction);
  if (std::abs(denominator) < k_epsilon) {
    return false;
  }

  const float t = (pn.d - glm::dot(pn.normal, origin)) / denominator;
  if (t < 0.0f) {
    return false;
  }

  out_t = t;
  return true;
}

bool ray_aabb(const ray& ry, const aabb& box, float& out_t) {
  if (!ry.is_valid() || !box.is_valid()) {
    return false;
  }

  const glm::vec3& origin = ry.origin();
  const glm::vec3& direction = ry.direction();
  const glm::vec3& box_min = box.min();
  const glm::vec3& box_max = box.max();
  float t_min = 0.0f;
  float t_max = std::numeric_limits<float>::infinity();

  for (int axis = 0; axis < 3; ++axis) {
    if (std::abs(direction[axis]) < k_epsilon) {
      if (origin[axis] < box_min[axis] || origin[axis] > box_max[axis]) {
        return false;
      }
      continue;
    }

    const float inverse = 1.0f / direction[axis];
    float t0 = (box_min[axis] - origin[axis]) * inverse;
    float t1 = (box_max[axis] - origin[axis]) * inverse;
    if (t0 > t1) {
      std::swap(t0, t1);
    }

    t_min = std::max(t_min, t0);
    t_max = std::min(t_max, t1);
    if (t_min > t_max) {
      return false;
    }
  }

  out_t = t_min;
  return true;
}

bool ray_bounding_sphere(const ray& ry, const bounding_sphere& sphere, float& out_t) {
  if (!ry.is_valid() || !sphere.is_valid()) {
    return false;
  }

  const glm::vec3 offset = ry.origin() - sphere.center();
  const float b = glm::dot(offset, ry.direction());
  const float c = glm::dot(offset, offset) - sphere.radius() * sphere.radius();
  const float discriminant = b * b - c;
  if (discriminant < 0.0f) {
    return false;
  }

  const float root = std::sqrt(discriminant);
  const float t0 = -b - root;
  const float t1 = -b + root;
  if (t1 < 0.0f) {
    return false;
  }

  out_t = (t0 >= 0.0f) ? t0 : t1;
  return true;
}

bool ray_triangle(const ray& ry, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& out_t) {
  if (!ry.is_valid()) {
    return false;
  }
  return ray_triangle_impl(ry.origin(), ry.direction(), v0, v1, v2, out_t);
}

bool ray_mesh(const ray& ry, const mesh_data& mesh, float& out_t, uint32_t& out_triangle_index) {
  const std::vector<vertex>& vertices = mesh.vertices();
  const std::vector<uint32_t>& indices = mesh.indices();
  if (!ry.is_valid() || vertices.empty() || indices.size() < 3) {
    return false;
  }

  const glm::vec3& origin = ry.origin();
  const glm::vec3& direction = ry.direction();
  float nearest = std::numeric_limits<float>::infinity();
  uint32_t nearest_triangle = 0;
  bool hit = false;

  for (size_t i = 0; i + 2 < indices.size(); i += 3) {
    const uint32_t i0 = indices[i];
    const uint32_t i1 = indices[i + 1];
    const uint32_t i2 = indices[i + 2];
    if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) {
      continue;
    }

    float t = 0.0f;
    if (!ray_triangle_impl(origin, direction, vertices[i0].position, vertices[i1].position, vertices[i2].position, t)) {
      continue;
    }

    if (t < nearest) {
      nearest = t;
      nearest_triangle = static_cast<uint32_t>(i / 3);
      hit = true;
    }
  }

  if (!hit) {
    return false;
  }

  out_t = nearest;
  out_triangle_index = nearest_triangle;
  return true;
}

} // namespace geometry
} // namespace cyberverse
