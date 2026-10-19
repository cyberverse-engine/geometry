/**
 * @file vertex.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Vertex definition
 * @version 0.1
 * @date 2026-10-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace cyberverse {
namespace geometry {

/**
 * @brief Vertex definition
 *
 */
struct vertex {
  /**
   * @brief Position
   *
   */
  glm::vec3 position;
  /**
   * @brief Normal
   *
   */
  glm::vec3 normal;
  /**
   * @brief UV
   *
   */
  glm::vec2 uv;
};

} // namespace geometry
} // namespace cyberverse
