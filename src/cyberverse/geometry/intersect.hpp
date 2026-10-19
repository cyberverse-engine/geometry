/**
 * @file intersect.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Intersect algorithms
 * @version 0.1
 * @date 2026-10-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <glm/vec3.hpp>

#include "cyberverse/geometry/bounds.hpp"

namespace cyberverse {
namespace geometry {

/**
 * @brief Ray with a unit-length direction
 *
 * Constructing a ray from an arbitrary non-zero direction normalizes it, so
 * intersection results are distances along the ray. A default-constructed ray,
 * or one built from a zero or non-finite direction, is invalid and misses every
 * intersection test.
 */
class ray {
public:
  ray() = default;

  /**
   * @brief Construct from an origin and a direction
   *
   * The direction is normalized. Zero or non-finite input produces an invalid ray.
   *
   * @param origin
   * @param direction
   */
  ray(const glm::vec3& origin, const glm::vec3& direction) noexcept;

  /**
   * @brief Construct from an origin and an already unit-length direction
   *
   * Skips normalization. Debug builds assert that the direction is unit length.
   * In release builds is_valid() still rejects grossly non-unit directions.
   *
   * @param origin
   * @param unit_direction
   * @return ray
   */
  static ray from_normalized(const glm::vec3& origin, const glm::vec3& unit_direction) noexcept;

  const glm::vec3& origin() const noexcept;

  /**
   * @brief Unit-length direction
   *
   * @return const glm::vec3&
   */
  const glm::vec3& direction() const noexcept;

  /**
   * @brief Is the ray usable for intersection tests
   *
   * @return true when the origin is finite and the direction has unit length
   */
  bool is_valid() const noexcept;

private:
  glm::vec3 origin_{ 0.0f };
  glm::vec3 direction_{ 0.0f };
};

struct plane {
  glm::vec3 normal; // 单位长度
  float d;          // plane: dot(normal, p) = d
};

/**
 * @brief Ray-plane intersection
 *
 * @param ry
 * @param pn
 * @param out_t 沿单位方向的命中距离，命中时 t >= 0
 * @return false when the ray is invalid, parallel to the plane, or hits behind the origin
 */
bool ray_plane(const ray& ry, const plane& pn, float& out_t);

/**
 * @brief Ray-AABB intersection
 *
 * @param ry
 * @param box
 * @param out_t 最近命中距离，命中时 t >= 0；origin 在盒内时为 0
 * @return false when the ray is invalid or misses
 */
bool ray_aabb(const ray& ry, const aabb& box, float& out_t);

/**
 * @brief Ray-bounding-sphere intersection
 *
 * @param ry
 * @param sphere
 * @param out_t 最近命中距离，命中时 t >= 0；origin 在球内时为出射距离
 * @return false when the ray is invalid or misses
 */
bool ray_bounding_sphere(const ray& ry, const bounding_sphere& sphere, float& out_t);

/**
 * @brief Ray-triangle intersection (double-sided)
 *
 * @param ry
 * @param v0
 * @param v1
 * @param v2
 * @param out_t 命中距离，命中时 t >= 0
 * @return false when the ray is invalid, parallel to the triangle, or misses
 */
bool ray_triangle(const ray& ry, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& out_t);

/**
 * @brief 对 MeshData 做逐三角形求交，返回最近命中的距离和三角形索引
 *
 * @param ry
 * @param mesh
 * @param out_t 最近命中的距离，命中时 t >= 0
 * @param out_triangle_index 命中的三角形索引
 * @return bool
 */
bool ray_mesh(const ray& ry, const mesh_data& mesh, float& out_t, uint32_t& out_triangle_index);

} // namespace geometry

} // namespace cyberverse
