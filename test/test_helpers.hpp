#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include <glm/geometric.hpp>

#include "cyberverse/geometry/mesh_data.hpp"

namespace test_support {

using cyberverse::geometry::mesh_data;
using cyberverse::geometry::vertex;

inline float relative_epsilon(float scale) { return 1e-4f * std::max(1.0f, std::abs(scale)); }

inline bool indices_are_valid(const mesh_data& mesh) {
  if (mesh.indices().size() % 3 != 0) {
    return false;
  }
  for (const uint32_t index : mesh.indices()) {
    if (index >= mesh.vertices().size()) {
      return false;
    }
  }
  return true;
}

inline bool positions_on_sphere(const mesh_data& mesh, float radius) {
  const float epsilon = relative_epsilon(radius);
  for (const vertex& v : mesh.vertices()) {
    if (std::abs(glm::length(v.position) - radius) > epsilon) {
      return false;
    }
  }
  return true;
}

inline bool normals_are_unit(const mesh_data& mesh, float epsilon = 1e-4f) {
  for (const vertex& v : mesh.vertices()) {
    if (std::abs(glm::length(v.normal) - 1.0f) > epsilon) {
      return false;
    }
  }
  return true;
}

inline bool normals_point_outward(const mesh_data& mesh) {
  for (const vertex& v : mesh.vertices()) {
    if (glm::dot(v.normal, v.position) <= 0.0f) {
      return false;
    }
  }
  return true;
}

inline bool has_no_degenerate_triangles(const mesh_data& mesh, float scale = 1.0f) {
  const float threshold = 1e-6f * std::max(1.0f, scale * scale);
  for (size_t i = 0; i + 2 < mesh.indices().size(); i += 3) {
    const glm::vec3& v0 = mesh.vertices()[mesh.indices()[i]].position;
    const glm::vec3& v1 = mesh.vertices()[mesh.indices()[i + 1]].position;
    const glm::vec3& v2 = mesh.vertices()[mesh.indices()[i + 2]].position;
    if (glm::length(glm::cross(v1 - v0, v2 - v0)) <= threshold) {
      return false;
    }
  }
  return true;
}

inline bool triangles_face_outward(const mesh_data& mesh, const glm::vec3& interior) {
  for (size_t i = 0; i + 2 < mesh.indices().size(); i += 3) {
    const glm::vec3& v0 = mesh.vertices()[mesh.indices()[i]].position;
    const glm::vec3& v1 = mesh.vertices()[mesh.indices()[i + 1]].position;
    const glm::vec3& v2 = mesh.vertices()[mesh.indices()[i + 2]].position;
    const glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);
    const glm::vec3 centroid = (v0 + v1 + v2) / 3.0f - interior;
    if (glm::dot(normal, centroid) <= 0.0f) {
      return false;
    }
  }
  return true;
}

inline bool uvs_in_unit_range(const mesh_data& mesh, float epsilon = 1e-4f) {
  for (const vertex& v : mesh.vertices()) {
    if (v.uv.x < -epsilon || v.uv.x > 1.0f + epsilon || v.uv.y < -epsilon || v.uv.y > 1.0f + epsilon) {
      return false;
    }
  }
  return true;
}

inline bool ico_uv_spans_ok(const mesh_data& mesh, float epsilon = 1e-4f) {
  for (size_t i = 0; i + 2 < mesh.indices().size(); i += 3) {
    const vertex& v0 = mesh.vertices()[mesh.indices()[i]];
    const vertex& v1 = mesh.vertices()[mesh.indices()[i + 1]];
    const vertex& v2 = mesh.vertices()[mesh.indices()[i + 2]];
    const float max_u = std::max({ v0.uv.x, v1.uv.x, v2.uv.x });
    const float min_u = std::min({ v0.uv.x, v1.uv.x, v2.uv.x });
    if (max_u - min_u > 0.5f + epsilon) {
      return false;
    }
    if (v0.uv.y < -epsilon || v0.uv.y > 1.0f + epsilon || v1.uv.y < -epsilon || v1.uv.y > 1.0f + epsilon ||
        v2.uv.y < -epsilon || v2.uv.y > 1.0f + epsilon) {
      return false;
    }
  }
  return true;
}

inline size_t unused_vertex_count(const mesh_data& mesh) {
  std::vector<bool> used(mesh.vertex_count(), false);
  for (const uint32_t index : mesh.indices()) {
    if (index < used.size()) {
      used[index] = true;
    }
  }

  size_t unused = 0;
  for (const bool value : used) {
    if (!value) {
      ++unused;
    }
  }
  return unused;
}

inline float surface_area(const mesh_data& mesh) {
  float area = 0.0f;
  for (size_t i = 0; i + 2 < mesh.indices().size(); i += 3) {
    const glm::vec3& v0 = mesh.vertices()[mesh.indices()[i]].position;
    const glm::vec3& v1 = mesh.vertices()[mesh.indices()[i + 1]].position;
    const glm::vec3& v2 = mesh.vertices()[mesh.indices()[i + 2]].position;
    area += 0.5f * glm::length(glm::cross(v1 - v0, v2 - v0));
  }
  return area;
}

inline bool flat_normals_consistent(const mesh_data& mesh, float epsilon = 1e-4f) {
  for (size_t i = 0; i + 2 < mesh.indices().size(); i += 3) {
    const vertex* tri[3] = { &mesh.vertices()[mesh.indices()[i]],
                             &mesh.vertices()[mesh.indices()[i + 1]],
                             &mesh.vertices()[mesh.indices()[i + 2]] };
    const glm::vec3 normal = glm::cross(tri[1]->position - tri[0]->position, tri[2]->position - tri[0]->position);
    const float length = glm::length(normal);
    if (length <= 0.0f) {
      return false;
    }
    const glm::vec3 face_normal = normal / length;
    for (const vertex* v : tri) {
      if (glm::length(v->normal - face_normal) > epsilon) {
        return false;
      }
    }
  }
  return true;
}

inline bool meshes_equal(const mesh_data& lhs, const mesh_data& rhs, float epsilon = 1e-5f) {
  if (lhs.vertex_count() != rhs.vertex_count() || lhs.indices() != rhs.indices()) {
    return false;
  }
  for (size_t i = 0; i < lhs.vertex_count(); ++i) {
    const vertex& a = lhs.vertices()[i];
    const vertex& b = rhs.vertices()[i];
    if (glm::length(a.position - b.position) > epsilon || glm::length(a.normal - b.normal) > epsilon ||
        std::abs(a.uv.x - b.uv.x) > epsilon || std::abs(a.uv.y - b.uv.y) > epsilon) {
      return false;
    }
  }
  return true;
}

inline std::vector<uint32_t> weld_vertex_ids(const mesh_data& mesh, float tolerance) {
  const float cell = std::max(tolerance, 1e-6f);
  const auto cell_of = [cell](const glm::vec3& point) {
    return std::array<long long, 3>{ static_cast<long long>(std::llround(point.x / cell)),
                                     static_cast<long long>(std::llround(point.y / cell)),
                                     static_cast<long long>(std::llround(point.z / cell)) };
  };
  const uint32_t missing = std::numeric_limits<uint32_t>::max();

  std::map<std::array<long long, 3>, std::vector<uint32_t>> buckets;
  std::vector<glm::vec3> positions;
  std::vector<uint32_t> ids(mesh.vertex_count(), missing);

  for (size_t i = 0; i < mesh.vertices().size(); ++i) {
    const glm::vec3& position = mesh.vertices()[i].position;
    const std::array<long long, 3> key = cell_of(position);
    uint32_t found = missing;

    for (long long dx = -1; dx <= 1 && found == missing; ++dx) {
      for (long long dy = -1; dy <= 1 && found == missing; ++dy) {
        for (long long dz = -1; dz <= 1 && found == missing; ++dz) {
          const std::array<long long, 3> neighbor{ key[0] + dx, key[1] + dy, key[2] + dz };
          const auto bucket = buckets.find(neighbor);
          if (bucket == buckets.end()) {
            continue;
          }
          for (const uint32_t id : bucket->second) {
            if (glm::length(positions[id] - position) <= tolerance) {
              found = id;
              break;
            }
          }
        }
      }
    }

    if (found == missing) {
      found = static_cast<uint32_t>(positions.size());
      positions.push_back(position);
      buckets[key].push_back(found);
    }
    ids[i] = found;
  }

  return ids;
}

struct topology_summary {
  size_t vertex_count = 0;
  size_t edge_count = 0;
  size_t boundary_edges = 0;
  size_t non_manifold_edges = 0;
  long long euler_characteristic = 0;
  bool valid = false;
};

inline topology_summary analyze_topology(const mesh_data& mesh, float tolerance) {
  topology_summary summary;
  if (!indices_are_valid(mesh)) {
    return summary;
  }

  const std::vector<uint32_t> ids = weld_vertex_ids(mesh, tolerance);
  std::map<std::pair<uint32_t, uint32_t>, size_t> edges;
  std::set<uint32_t> used;

  for (size_t i = 0; i + 2 < mesh.indices().size(); i += 3) {
    const uint32_t triangle[3] = { ids[mesh.indices()[i]], ids[mesh.indices()[i + 1]], ids[mesh.indices()[i + 2]] };
    for (int k = 0; k < 3; ++k) {
      used.insert(triangle[k]);
      const uint32_t a = std::min(triangle[k], triangle[(k + 1) % 3]);
      const uint32_t b = std::max(triangle[k], triangle[(k + 1) % 3]);
      ++edges[{ a, b }];
    }
  }

  summary.vertex_count = used.size();
  summary.edge_count = edges.size();
  for (const auto& entry : edges) {
    const size_t count = entry.second;
    if (count == 1) {
      ++summary.boundary_edges;
    } else if (count > 2) {
      ++summary.non_manifold_edges;
    }
  }
  summary.euler_characteristic = static_cast<long long>(summary.vertex_count) -
                                 static_cast<long long>(summary.edge_count) +
                                 static_cast<long long>(mesh.triangle_count());
  summary.valid = true;
  return summary;
}

} // namespace test_support
