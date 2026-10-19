/**
 * @file primitives.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Primitives
 * @version 0.1
 * @date 2026-10-19
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/geometry/primitives.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>

#include <glm/geometric.hpp>

namespace cyberverse {
namespace geometry {
namespace {

constexpr float k_pi = 3.14159265358979323846f;
constexpr float k_normal_epsilon = 1e-20f;
constexpr int k_ico_max_subdivisions = 8;
constexpr int k_quad_max_resolution = 256;

uint32_t append_vertex(mesh_data& mesh, const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv) {
  const auto index = static_cast<uint32_t>(mesh.vertices().size());
  mesh.vertices().push_back(vertex{ position, normal, uv });
  return index;
}

void append_triangle(mesh_data& mesh, uint32_t a, uint32_t b, uint32_t c) {
  mesh.indices().push_back(a);
  mesh.indices().push_back(b);
  mesh.indices().push_back(c);
}

void append_quad(mesh_data& mesh,
                 const glm::vec3& p0,
                 const glm::vec3& p1,
                 const glm::vec3& p2,
                 const glm::vec3& p3,
                 const glm::vec3& normal) {
  const uint32_t i0 = append_vertex(mesh, p0, normal, { 0.0f, 0.0f });
  const uint32_t i1 = append_vertex(mesh, p1, normal, { 1.0f, 0.0f });
  const uint32_t i2 = append_vertex(mesh, p2, normal, { 1.0f, 1.0f });
  const uint32_t i3 = append_vertex(mesh, p3, normal, { 0.0f, 1.0f });
  append_triangle(mesh, i0, i1, i2);
  append_triangle(mesh, i0, i2, i3);
}

/**
 * @brief Replace smooth vertex sharing with one constant normal per triangle
 *
 * Positions and UVs are preserved, so this only changes the normal layout.
 */
mesh_data flatten(const mesh_data& source) {
  mesh_data result;
  result.reserve(source.indices().size(), source.indices().size());

  const std::vector<vertex>& vertices = source.vertices();
  const std::vector<uint32_t>& indices = source.indices();
  for (size_t i = 0; i + 2 < indices.size(); i += 3) {
    const uint32_t i0 = indices[i];
    const uint32_t i1 = indices[i + 1];
    const uint32_t i2 = indices[i + 2];
    if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) {
      continue;
    }

    const vertex& v0 = vertices[i0];
    const vertex& v1 = vertices[i1];
    const vertex& v2 = vertices[i2];

    glm::vec3 normal = glm::cross(v1.position - v0.position, v2.position - v0.position);
    const float length_squared = glm::dot(normal, normal);
    normal = (length_squared > k_normal_epsilon) ? normal / std::sqrt(length_squared) : glm::vec3(0.0f, 1.0f, 0.0f);

    const uint32_t base = static_cast<uint32_t>(result.vertices().size());
    result.vertices().push_back(vertex{ v0.position, normal, v0.uv });
    result.vertices().push_back(vertex{ v1.position, normal, v1.uv });
    result.vertices().push_back(vertex{ v2.position, normal, v2.uv });
    append_triangle(result, base, base + 1, base + 2);
  }

  return result;
}

mesh_data create_uv_sphere(float radius, int rings, int segments) {
  mesh_data mesh;
  const int columns = segments + 1;
  const float ring_step = k_pi / static_cast<float>(rings);
  const float angle_step = 2.0f * k_pi / static_cast<float>(segments);

  mesh.reserve(static_cast<size_t>(2) + static_cast<size_t>(rings - 1) * static_cast<size_t>(columns),
               static_cast<size_t>(2) * static_cast<size_t>(segments) * static_cast<size_t>(rings - 1) * 3);

  const uint32_t north = append_vertex(mesh, { 0.0f, radius, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.5f, 1.0f });

  const uint32_t body_base = static_cast<uint32_t>(mesh.vertices().size());
  for (int ring = 1; ring < rings; ++ring) {
    const float phi = ring_step * static_cast<float>(ring);
    const float sin_phi = std::sin(phi);
    const float cos_phi = std::cos(phi);
    for (int segment = 0; segment < columns; ++segment) {
      const float angle = angle_step * static_cast<float>(segment);
      const glm::vec3 direction{ sin_phi * std::cos(angle), cos_phi, sin_phi * std::sin(angle) };
      append_vertex(mesh,
                    direction * radius,
                    direction,
                    { static_cast<float>(segment) / static_cast<float>(segments),
                      1.0f - static_cast<float>(ring) / static_cast<float>(rings) });
    }
  }

  const uint32_t south = append_vertex(mesh, { 0.0f, -radius, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.0f });

  const auto row = [body_base, columns](int ring, int segment) {
    return body_base + static_cast<uint32_t>((ring - 1) * columns + segment);
  };

  for (int segment = 0; segment < segments; ++segment) {
    append_triangle(mesh, north, row(1, segment + 1), row(1, segment));
  }
  for (int ring = 1; ring < rings - 1; ++ring) {
    for (int segment = 0; segment < segments; ++segment) {
      const uint32_t a = row(ring, segment);
      const uint32_t b = row(ring, segment + 1);
      const uint32_t c = row(ring + 1, segment);
      const uint32_t d = row(ring + 1, segment + 1);
      append_triangle(mesh, a, b, c);
      append_triangle(mesh, b, d, c);
    }
  }
  for (int segment = 0; segment < segments; ++segment) {
    append_triangle(mesh, row(rings - 1, segment), row(rings - 1, segment + 1), south);
  }

  return mesh;
}

struct ico_face {
  uint32_t a;
  uint32_t b;
  uint32_t c;
};

std::vector<glm::vec3> make_ico_directions() {
  const float phi = (1.0f + std::sqrt(5.0f)) * 0.5f;
  const glm::vec3 raw[12] = {
    { -1.0f, phi, 0.0f }, { 1.0f, phi, 0.0f }, { -1.0f, -phi, 0.0f }, { 1.0f, -phi, 0.0f },
    { 0.0f, -1.0f, phi }, { 0.0f, 1.0f, phi }, { 0.0f, -1.0f, -phi }, { 0.0f, 1.0f, -phi },
    { phi, 0.0f, -1.0f }, { phi, 0.0f, 1.0f }, { -phi, 0.0f, -1.0f }, { -phi, 0.0f, 1.0f },
  };

  std::vector<glm::vec3> directions;
  directions.reserve(12);
  for (const glm::vec3& direction : raw) {
    directions.push_back(glm::normalize(direction));
  }
  return directions;
}

std::vector<ico_face> make_ico_faces(const std::vector<glm::vec3>& directions) {
  std::vector<ico_face> faces = {
    { 0, 11, 5 },  { 0, 5, 1 },  { 0, 1, 7 },  { 0, 7, 10 }, { 0, 10, 11 }, { 1, 5, 9 }, { 5, 11, 4 },
    { 11, 10, 2 }, { 10, 7, 6 }, { 7, 1, 8 },  { 3, 9, 4 },  { 3, 4, 2 },   { 3, 2, 6 }, { 3, 6, 8 },
    { 3, 8, 9 },   { 4, 9, 5 },  { 2, 4, 11 }, { 6, 2, 10 }, { 8, 6, 7 },   { 9, 8, 1 },
  };

  for (ico_face& face : faces) {
    const glm::vec3& v0 = directions[face.a];
    const glm::vec3& v1 = directions[face.b];
    const glm::vec3& v2 = directions[face.c];
    if (glm::dot(glm::cross(v1 - v0, v2 - v0), v0 + v1 + v2) < 0.0f) {
      std::swap(face.b, face.c);
    }
  }
  return faces;
}

uint32_t ico_midpoint(std::vector<glm::vec3>& directions,
                      std::unordered_map<uint64_t, uint32_t>& cache,
                      uint32_t a,
                      uint32_t b) {
  const uint32_t low = std::min(a, b);
  const uint32_t high = std::max(a, b);
  const uint64_t key = (static_cast<uint64_t>(low) << 32) | static_cast<uint64_t>(high);
  const auto found = cache.find(key);
  if (found != cache.end()) {
    return found->second;
  }

  const glm::vec3 midpoint = glm::normalize(directions[a] + directions[b]);
  const uint32_t index = static_cast<uint32_t>(directions.size());
  directions.push_back(midpoint);
  cache.emplace(key, index);
  return index;
}

mesh_data create_ico_sphere(float radius, int subdivisions) {
  std::vector<glm::vec3> directions = make_ico_directions();
  std::vector<ico_face> faces = make_ico_faces(directions);

  size_t edge_count = 30;
  for (int level = 0; level < subdivisions; ++level) {
    edge_count *= 4;
  }
  std::unordered_map<uint64_t, uint32_t> midpoint_cache;
  midpoint_cache.reserve(edge_count);

  for (int level = 0; level < subdivisions; ++level) {
    std::vector<ico_face> next;
    next.reserve(faces.size() * 4);
    for (const ico_face& face : faces) {
      const uint32_t ab = ico_midpoint(directions, midpoint_cache, face.a, face.b);
      const uint32_t bc = ico_midpoint(directions, midpoint_cache, face.b, face.c);
      const uint32_t ca = ico_midpoint(directions, midpoint_cache, face.c, face.a);
      next.push_back({ face.a, ab, ca });
      next.push_back({ ab, face.b, bc });
      next.push_back({ ca, bc, face.c });
      next.push_back({ ab, bc, ca });
    }
    faces = std::move(next);
  }

  std::vector<vertex> canonical;
  canonical.reserve(directions.size());
  for (const glm::vec3& direction : directions) {
    float u = std::atan2(direction.z, direction.x) / (2.0f * k_pi);
    if (u < 0.0f) {
      u += 1.0f;
    }
    const float v = 0.5f + std::asin(std::clamp(direction.y, -1.0f, 1.0f)) / k_pi;
    canonical.push_back(vertex{ direction * radius, direction, { u, v } });
  }

  mesh_data mesh(std::move(canonical), std::vector<uint32_t>{});
  mesh.reserve(mesh.vertex_count(), faces.size() * 3);

  for (const ico_face& face : faces) {
    const float raw_u[3] = { mesh.vertices()[face.a].uv.x, mesh.vertices()[face.b].uv.x, mesh.vertices()[face.c].uv.x };
    int order[3] = { 0, 1, 2 };
    std::sort(order, order + 3, [&raw_u](int lhs, int rhs) {
      return raw_u[lhs] < raw_u[rhs];
    });

    const float low = raw_u[order[0]];
    const float middle = raw_u[order[1]];
    const float high = raw_u[order[2]];
    const float gaps[3] = { middle - low, high - middle, 1.0f - high + low };
    int cut = 0;
    if (gaps[1] > gaps[cut]) {
      cut = 1;
    }
    if (gaps[2] > gaps[cut]) {
      cut = 2;
    }

    float u[3] = { raw_u[0], raw_u[1], raw_u[2] };
    if (cut == 0) {
      u[order[0]] += 1.0f;
    } else if (cut == 1) {
      u[order[0]] += 1.0f;
      u[order[1]] += 1.0f;
    }

    uint32_t indices[3] = { face.a, face.b, face.c };
    for (int k = 0; k < 3; ++k) {
      if (u[k] != raw_u[k]) {
        vertex duplicate = mesh.vertices()[indices[k]];
        duplicate.uv.x = u[k];
        indices[k] = static_cast<uint32_t>(mesh.vertices().size());
        mesh.vertices().push_back(duplicate);
      }
    }
    append_triangle(mesh, indices[0], indices[1], indices[2]);
  }

  return mesh;
}

struct quad_face {
  glm::vec3 normal;
  glm::vec3 u;
  glm::vec3 v;
};

glm::vec3 spherify_cube(const glm::vec3& point) {
  const auto component = [](float a, float b, float c) {
    const float value = 1.0f - 0.5f * b * b - 0.5f * c * c + (b * b * c * c) / 3.0f;
    return a * std::sqrt(std::max(0.0f, value));
  };
  return { component(point.x, point.y, point.z),
           component(point.y, point.z, point.x),
           component(point.z, point.x, point.y) };
}

mesh_data create_quad_sphere(float radius, int resolution) {
  const quad_face faces[6] = {
    { { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
    { { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    { { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 0.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
  };

  mesh_data mesh;
  const int side = resolution + 1;
  const float step = 2.0f / static_cast<float>(resolution);
  mesh.reserve(static_cast<size_t>(6) * static_cast<size_t>(side) * static_cast<size_t>(side),
               static_cast<size_t>(6) * static_cast<size_t>(resolution) * static_cast<size_t>(resolution) * 6);

  for (const quad_face& face : faces) {
    const uint32_t base = static_cast<uint32_t>(mesh.vertices().size());
    for (int j = 0; j < side; ++j) {
      const float t = -1.0f + step * static_cast<float>(j);
      for (int i = 0; i < side; ++i) {
        const float s = -1.0f + step * static_cast<float>(i);
        const glm::vec3 cube = face.normal + face.u * s + face.v * t;
        const glm::vec3 direction = glm::normalize(spherify_cube(cube));
        append_vertex(mesh, direction * radius, direction, { (s + 1.0f) * 0.5f, (t + 1.0f) * 0.5f });
      }
    }

    for (int j = 0; j < resolution; ++j) {
      for (int i = 0; i < resolution; ++i) {
        const uint32_t i0 = base + static_cast<uint32_t>(j * side + i);
        const uint32_t i1 = i0 + 1;
        const uint32_t i2 = i0 + static_cast<uint32_t>(side);
        const uint32_t i3 = i2 + 1;
        append_triangle(mesh, i0, i1, i3);
        append_triangle(mesh, i0, i3, i2);
      }
    }
  }

  return mesh;
}

mesh_data make_unit_quad() {
  mesh_data mesh;
  mesh.reserve(4, 6);
  append_quad(mesh,
              { -0.5f, 0.0f, -0.5f },
              { -0.5f, 0.0f, 0.5f },
              { 0.5f, 0.0f, 0.5f },
              { 0.5f, 0.0f, -0.5f },
              { 0.0f, 1.0f, 0.0f });
  return mesh;
}

} // namespace

const mesh_data primitive_unit_quad = make_unit_quad();

mesh_data create_unit_slab(float thickness) { return create_box(1.0f, 1.0f, thickness); }

mesh_data create_box(float width, float height, float depth) {
  mesh_data mesh;
  if (width <= 0.0f || height <= 0.0f || depth <= 0.0f) {
    return mesh;
  }

  const float hx = width * 0.5f;
  const float hy = height * 0.5f;
  const float hz = depth * 0.5f;
  mesh.reserve(24, 36);

  append_quad(mesh, { hx, -hy, -hz }, { hx, hy, -hz }, { hx, hy, hz }, { hx, -hy, hz }, { 1.0f, 0.0f, 0.0f });
  append_quad(mesh, { -hx, -hy, hz }, { -hx, hy, hz }, { -hx, hy, -hz }, { -hx, -hy, -hz }, { -1.0f, 0.0f, 0.0f });
  append_quad(mesh, { -hx, hy, hz }, { hx, hy, hz }, { hx, hy, -hz }, { -hx, hy, -hz }, { 0.0f, 1.0f, 0.0f });
  append_quad(mesh, { -hx, -hy, -hz }, { hx, -hy, -hz }, { hx, -hy, hz }, { -hx, -hy, hz }, { 0.0f, -1.0f, 0.0f });
  append_quad(mesh, { -hx, -hy, hz }, { hx, -hy, hz }, { hx, hy, hz }, { -hx, hy, hz }, { 0.0f, 0.0f, 1.0f });
  append_quad(mesh, { hx, -hy, -hz }, { -hx, -hy, -hz }, { -hx, hy, -hz }, { hx, hy, -hz }, { 0.0f, 0.0f, -1.0f });

  return mesh;
}

const mesh_data primitive_unit_cube = create_box(1.0f, 1.0f, 1.0f);

mesh_data create_cylinder(float radius, float height, const cylinder_options& opts) {
  mesh_data mesh;
  if (radius <= 0.0f || height <= 0.0f) {
    return mesh;
  }

  const int segments = std::max(opts.segments, 3);
  const float half_height = height * 0.5f;
  const float angle_step = 2.0f * k_pi / static_cast<float>(segments);

  mesh.reserve(4 * (segments + 1) + 2, 12 * segments);

  const uint32_t side_base = static_cast<uint32_t>(mesh.vertices().size());
  for (int i = 0; i <= segments; ++i) {
    const float angle = angle_step * static_cast<float>(i);
    const float x = radius * std::cos(angle);
    const float z = radius * std::sin(angle);
    const glm::vec3 normal{ std::cos(angle), 0.0f, std::sin(angle) };
    append_vertex(mesh, { x, -half_height, z }, normal, { static_cast<float>(i) / segments, 0.0f });
    append_vertex(mesh, { x, half_height, z }, normal, { static_cast<float>(i) / segments, 1.0f });
  }

  for (int i = 0; i < segments; ++i) {
    const uint32_t b0 = side_base + static_cast<uint32_t>(2 * i);
    const uint32_t t0 = b0 + 1;
    const uint32_t b1 = b0 + 2;
    const uint32_t t1 = b1 + 1;
    append_triangle(mesh, b0, t0, t1);
    append_triangle(mesh, b0, t1, b1);
  }

  if (opts.caps) {
    const uint32_t top_center = append_vertex(mesh, { 0.0f, half_height, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.5f, 0.5f });
    const uint32_t top_ring = static_cast<uint32_t>(mesh.vertices().size());
    for (int i = 0; i <= segments; ++i) {
      const float angle = angle_step * static_cast<float>(i);
      const float x = radius * std::cos(angle);
      const float z = radius * std::sin(angle);
      append_vertex(mesh,
                    { x, half_height, z },
                    { 0.0f, 1.0f, 0.0f },
                    { 0.5f + 0.5f * std::cos(angle), 0.5f + 0.5f * std::sin(angle) });
    }
    for (int i = 0; i < segments; ++i) {
      append_triangle(mesh, top_center, top_ring + static_cast<uint32_t>(i + 1), top_ring + static_cast<uint32_t>(i));
    }

    const uint32_t bottom_center =
        append_vertex(mesh, { 0.0f, -half_height, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.5f });
    const uint32_t bottom_ring = static_cast<uint32_t>(mesh.vertices().size());
    for (int i = 0; i <= segments; ++i) {
      const float angle = angle_step * static_cast<float>(i);
      const float x = radius * std::cos(angle);
      const float z = radius * std::sin(angle);
      append_vertex(mesh,
                    { x, -half_height, z },
                    { 0.0f, -1.0f, 0.0f },
                    { 0.5f + 0.5f * std::cos(angle), 0.5f - 0.5f * std::sin(angle) });
    }
    for (int i = 0; i < segments; ++i) {
      append_triangle(
          mesh, bottom_center, bottom_ring + static_cast<uint32_t>(i), bottom_ring + static_cast<uint32_t>(i + 1));
    }
  }

  return (opts.normal == normal_mode::flat) ? flatten(mesh) : mesh;
}

mesh_data create_cone(float radius, float height, int segments) {
  mesh_data mesh;
  if (radius <= 0.0f || height <= 0.0f) {
    return mesh;
  }

  segments = std::max(segments, 3);
  const float angle_step = 2.0f * k_pi / static_cast<float>(segments);
  const float slope_length = std::sqrt(height * height + radius * radius);

  mesh.reserve(3 * (segments + 1), 6 * segments);

  const uint32_t base_ring = static_cast<uint32_t>(mesh.vertices().size());
  for (int i = 0; i <= segments; ++i) {
    const float angle = angle_step * static_cast<float>(i);
    const float x = radius * std::cos(angle);
    const float z = radius * std::sin(angle);
    const glm::vec3 normal{ height * std::cos(angle) / slope_length,
                            radius / slope_length,
                            height * std::sin(angle) / slope_length };
    append_vertex(mesh, { x, 0.0f, z }, normal, { static_cast<float>(i) / segments, 0.0f });
  }

  for (int i = 0; i < segments; ++i) {
    const float middle_angle = angle_step * (static_cast<float>(i) + 0.5f);
    const glm::vec3 apex_normal{ height * std::cos(middle_angle) / slope_length,
                                 radius / slope_length,
                                 height * std::sin(middle_angle) / slope_length };
    const uint32_t apex =
        append_vertex(mesh, { 0.0f, height, 0.0f }, apex_normal, { (static_cast<float>(i) + 0.5f) / segments, 1.0f });
    append_triangle(mesh, base_ring + static_cast<uint32_t>(i), apex, base_ring + static_cast<uint32_t>(i + 1));
  }

  const uint32_t base_center = append_vertex(mesh, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.5f });
  const uint32_t cap_ring = static_cast<uint32_t>(mesh.vertices().size());
  for (int i = 0; i <= segments; ++i) {
    const float angle = angle_step * static_cast<float>(i);
    append_vertex(mesh,
                  { radius * std::cos(angle), 0.0f, radius * std::sin(angle) },
                  { 0.0f, -1.0f, 0.0f },
                  { 0.5f + 0.5f * std::cos(angle), 0.5f - 0.5f * std::sin(angle) });
  }
  for (int i = 0; i < segments; ++i) {
    append_triangle(mesh, base_center, cap_ring + static_cast<uint32_t>(i), cap_ring + static_cast<uint32_t>(i + 1));
  }

  return mesh;
}

mesh_data create_sphere(float radius, const sphere_options& opts) {
  if (!(radius > 0.0f) || !std::isfinite(radius)) {
    return {};
  }

  mesh_data mesh;
  switch (opts.kind) {
  case sphere_kind::ico: {
    const int subdivisions = std::clamp(opts.ico_subdivisions, 0, k_ico_max_subdivisions);
    mesh = create_ico_sphere(radius, subdivisions);
    break;
  }
  case sphere_kind::quad: {
    const int resolution = std::clamp(opts.quad_resolution, 1, k_quad_max_resolution);
    mesh = create_quad_sphere(radius, resolution);
    break;
  }
  case sphere_kind::uv:
  default: {
    const int rings = std::max(opts.rings, 2);
    const int segments = std::max(opts.segments, 3);
    mesh = create_uv_sphere(radius, rings, segments);
    break;
  }
  }

  return (opts.normal == normal_mode::flat) ? flatten(mesh) : mesh;
}

} // namespace geometry
} // namespace cyberverse
