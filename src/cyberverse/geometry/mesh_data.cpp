/**
 * @file mesh_data.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Mesh data implementation
 * @version 0.1
 * @date 2026-10-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/geometry/mesh_data.hpp"

#include <utility>

namespace cyberverse {
namespace geometry {

mesh_data::mesh_data(std::vector<vertex> vertices, std::vector<uint32_t> indices)
    : vertices_(std::move(vertices)), indices_(std::move(indices)) {}

size_t mesh_data::vertex_count() const noexcept { return vertices_.size(); }

size_t mesh_data::index_count() const noexcept { return indices_.size(); }

size_t mesh_data::triangle_count() const noexcept { return indices_.size() / 3; }

bool mesh_data::empty() const noexcept { return vertices_.empty(); }

mesh_data::operator bool() const noexcept { return !vertices_.empty(); }

const std::vector<vertex>& mesh_data::vertices() const noexcept { return vertices_; }

const std::vector<uint32_t>& mesh_data::indices() const noexcept { return indices_; }

std::vector<vertex>& mesh_data::vertices() noexcept { return vertices_; }

std::vector<uint32_t>& mesh_data::indices() noexcept { return indices_; }

void mesh_data::append(const mesh_data& other) {
  if (other.empty()) {
    return;
  }

  if (&other == this) {
    const mesh_data copy(other);
    append(copy);
    return;
  }

  const auto offset = static_cast<uint32_t>(vertices_.size());
  vertices_.insert(vertices_.end(), other.vertices_.begin(), other.vertices_.end());

  indices_.reserve(indices_.size() + other.indices_.size());
  for (const uint32_t index : other.indices_) {
    indices_.push_back(index + offset);
  }
}

void mesh_data::reserve(size_t vertex_count, size_t index_count) {
  vertices_.reserve(vertex_count);
  indices_.reserve(index_count);
}

void mesh_data::clear() noexcept {
  vertices_.clear();
  indices_.clear();
}

} // namespace geometry
} // namespace cyberverse
