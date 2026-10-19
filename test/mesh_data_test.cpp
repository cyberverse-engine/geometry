#include <catch2/catch_test_macros.hpp>

#include <utility>

#include "cyberverse/geometry/mesh_data.hpp"

using namespace cyberverse::geometry;

namespace {

mesh_data make_triangle(float y) {
  std::vector<vertex> vertices = {
    { { -1.0f, y, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { -1.0f, y, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
    { { 1.0f, y, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
  };
  return mesh_data(std::move(vertices), { 0, 1, 2 });
}

} // namespace

TEST_CASE("mesh_data default and explicit construction", "[mesh_data]") {
  const mesh_data empty;
  REQUIRE(empty.empty());
  REQUIRE_FALSE(empty);
  REQUIRE(empty.vertex_count() == 0);
  REQUIRE(empty.index_count() == 0);
  REQUIRE(empty.triangle_count() == 0);

  const mesh_data triangle = make_triangle(0.0f);
  REQUIRE_FALSE(triangle.empty());
  REQUIRE(triangle);
  REQUIRE(triangle.vertex_count() == 3);
  REQUIRE(triangle.index_count() == 3);
  REQUIRE(triangle.triangle_count() == 1);
}

TEST_CASE("mesh_data append offsets indices", "[mesh_data]") {
  mesh_data mesh = make_triangle(0.0f);
  const mesh_data other = make_triangle(1.0f);

  mesh.append(other);
  REQUIRE(mesh.vertex_count() == 6);
  REQUIRE(mesh.index_count() == 6);
  REQUIRE(mesh.triangle_count() == 2);
  REQUIRE(mesh.indices()[0] == 0);
  REQUIRE(mesh.indices()[3] == 3);
  REQUIRE(mesh.indices()[4] == 4);
  REQUIRE(mesh.indices()[5] == 5);
}

TEST_CASE("mesh_data append handles empty and self", "[mesh_data]") {
  mesh_data mesh = make_triangle(0.0f);
  mesh.append(mesh_data{});
  REQUIRE(mesh.vertex_count() == 3);
  REQUIRE(mesh.index_count() == 3);

  mesh.append(mesh);
  REQUIRE(mesh.vertex_count() == 6);
  REQUIRE(mesh.index_count() == 6);
  REQUIRE(mesh.indices()[0] == 0);
  REQUIRE(mesh.indices()[3] == 3);
  REQUIRE(mesh.vertices()[3].position.y == mesh.vertices()[0].position.y);
}

TEST_CASE("mesh_data reserve and clear", "[mesh_data]") {
  mesh_data mesh = make_triangle(0.0f);
  mesh.reserve(12, 36);
  REQUIRE(mesh.vertex_count() == 3);
  REQUIRE(mesh.index_count() == 3);

  mesh.clear();
  REQUIRE(mesh.empty());
  REQUIRE(mesh.vertex_count() == 0);
  REQUIRE(mesh.index_count() == 0);
  REQUIRE(mesh.triangle_count() == 0);
}

TEST_CASE("mesh_data empty is based on vertices, not indices", "[mesh_data]") {
  mesh_data vertices_only;
  vertices_only.vertices().push_back({ { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
  REQUIRE_FALSE(vertices_only.empty());
  REQUIRE(vertices_only.triangle_count() == 0);

  mesh_data partial_indices;
  partial_indices.vertices().push_back({ { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
  partial_indices.indices() = { 0, 0, 0, 0 };
  REQUIRE(partial_indices.triangle_count() == 1);
}
