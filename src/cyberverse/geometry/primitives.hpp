/**
 * @file primitives.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Primitive shapes
 * @version 0.1
 * @date 2026-10-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/geometry/mesh_data.hpp"

namespace cyberverse {
namespace geometry {

extern const mesh_data primitive_unit_quad;
extern const mesh_data primitive_unit_cube;

/**
 * @brief Normal generation mode
 *
 * smooth shares vertices across adjacent faces; flat duplicates them so each
 * triangle gets a constant face normal.
 */
enum class normal_mode { flat, smooth };

/**
 * @brief Unit quad
 *
 *   XZ 平面，y=0，范围 [-0.5,0.5]，法线 +Y
 *
 * @return const mesh_data&
 */
inline const mesh_data& unit_quad() noexcept { return primitive_unit_quad; }

/**
 * @brief Unit cube
 *
 *  1x1x1，中心在原点
 *
 * @return const mesh_data&
 */
inline const mesh_data& unit_cube() noexcept { return primitive_unit_cube; }

/**
 * @brief Create a unit slab object
 *
 *  1 x 1 x thickness，中心在原点，厚度沿 +Z
 *
 * @param thickness
 * @return mesh_data
 */
mesh_data create_unit_slab(float thickness);

/**
 * @brief Create a box object
 *
 *  中心在原点
 *
 * @param width  X 轴尺寸
 * @param height Y 轴尺寸
 * @param depth  Z 轴尺寸
 * @return mesh_data
 */
mesh_data create_box(float width, float height, float depth);

/**
 * @brief Cylinder options
 *
 */
struct cylinder_options {
  int segments = 12;
  normal_mode normal = normal_mode::smooth;
  bool caps = true;
};

/**
 * @brief Create a Cylinder object
 *
 *  轴向 +Y，沿 Y 轴居中（y 范围 [-height/2, height/2]）
 *
 * @param radius
 * @param height
 * @param opts
 * @return mesh_data
 */
mesh_data create_cylinder(float radius, float height, const cylinder_options& opts = {});

/**
 * @brief Create a cone object
 *
 *  底面中心在原点，轴向 +Y
 *
 * @param radius
 * @param height
 * @param segments
 * @return mesh_data
 */
mesh_data create_cone(float radius, float height, int segments = 12);

/**
 * @brief Sphere generation algorithm
 *
 * uv   - latitude/longitude grid; natural equirectangular UV, pinched poles.
 * ico  - subdivided icosahedron; near-uniform triangles, spherical UV with seam duplication.
 * quad - spherified cube; six face-local UV patches, no pole singularity.
 */
enum class sphere_kind { uv, ico, quad };

/**
 * @brief Sphere options
 *
 * Fields are shared by all algorithms; fields that do not apply to the selected
 * kind are ignored. A non-finite radius or radius <= 0 returns an empty mesh.
 *
 * uv:
 *   rings    latitude bands, clamped to >= 2
 *   segments longitude segments, clamped to >= 3
 *   triangles = 2 * segments * (rings - 1)
 *
 * ico:
 *   ico_subdivisions recursion levels, clamped to [0, 8]
 *   triangles = 20 * 4^ico_subdivisions
 *
 * quad:
 *   quad_resolution cells per cube face edge, clamped to [1, 256]
 *   triangles = 12 * quad_resolution^2
 */
struct sphere_options {
  int rings = 8;     //!< uv: latitude bands, >= 2
  int segments = 12; //!< uv: longitude segments, >= 3
  normal_mode normal = normal_mode::smooth;
  sphere_kind kind = sphere_kind::uv;
  int ico_subdivisions = 4; //!< ico: subdivision levels, 0..8
  int quad_resolution = 16; //!< quad: cells per cube face edge, 1..256
};

/**
 * @brief Create a sphere centered at the origin
 *
 * Dispatches to the algorithm selected by opts.kind. The default is the UV
 * sphere, preserving the original create_sphere() behavior. Triangles are
 * counter-clockwise when viewed from outside and vertex normals follow
 * opts.normal.
 *
 * @param radius
 * @param opts
 * @return mesh_data Empty when radius is not finite or <= 0
 */
mesh_data create_sphere(float radius, const sphere_options& opts = {});

} // namespace geometry
} // namespace cyberverse
