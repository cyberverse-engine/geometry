/**
 * @file mesh_data.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Mesh data definition
 * @version 0.1
 * @date 2026-10-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "cyberverse/geometry/vertex.hpp"

namespace cyberverse {
namespace geometry {

/**
 * @brief Mesh data definition
 *
 * Vertices are stored separately from indices; every three indices form one
 * triangle. A mesh is empty when it has no vertices; a vertex-only mesh has no
 * drawable triangles.
 */
class mesh_data {
public:
  mesh_data() = default;

  /**
   * @brief Construct from vertices and triangle indices
   *
   * @param vertices Vertex data
   * @param indices Three indices per triangle
   */
  mesh_data(std::vector<vertex> vertices, std::vector<uint32_t> indices);

  mesh_data(const mesh_data&) = default;
  mesh_data(mesh_data&&) noexcept = default;
  mesh_data& operator=(const mesh_data&) = default;
  mesh_data& operator=(mesh_data&&) noexcept = default;
  ~mesh_data() = default;

  /**
   * @brief Get vertex count
   *
   * @return size_t
   */
  size_t vertex_count() const noexcept;

  /**
   * @brief Get index count
   *
   * @return size_t
   */
  size_t index_count() const noexcept;

  /**
   * @brief Get triangle count
   *
   * @return size_t
   */
  size_t triangle_count() const noexcept;

  /**
   * @brief Is the mesh empty
   *
   * @return true when no vertices are present
   */
  bool empty() const noexcept;

  /**
   * @brief Is the mesh non-empty
   *
   * @return true when at least one vertex is present
   */
  explicit operator bool() const noexcept;

  const std::vector<vertex>& vertices() const noexcept;
  const std::vector<uint32_t>& indices() const noexcept;
  std::vector<vertex>& vertices() noexcept;
  std::vector<uint32_t>& indices() noexcept;

  /**
   * @brief Append another mesh data to current mesh
   *
   * Indices are offset so the appended mesh keeps its own topology.
   *
   * @param other
   */
  void append(const mesh_data& other);

  /**
   * @brief Reserve space
   *
   * @param vertex_count Number of vertex to reserve
   * @param index_count Number of index ot reserve
   */
  void reserve(size_t vertex_count, size_t index_count);

  /**
   * @brief Clear all data
   *
   */
  void clear() noexcept;

private:
  std::vector<vertex> vertices_;
  std::vector<uint32_t> indices_;
};

} // namespace geometry
} // namespace cyberverse
