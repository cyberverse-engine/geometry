# Geometry Roadmap

本文档记录 `geometry` 库的能力现状、功能缺口和优先级，供后续迭代参考。

## 当前能力

- `vertex` / `mesh_data`：位置、法线、UV，三角形索引，`append`、`reserve`、`clear`。
- `bounds`：`aabb`、`bounding_sphere`、`compute_aabb`、`compute_bounding_sphere`。
- `intersect`：ray-plane、ray-aabb、ray-bounding-sphere、ray-triangle、ray_mesh。`ray` 构造时归一化方向（已单位化时可用 `from_normalized` 跳过），`t` 为命中距离；无效 ray 返回 miss。
- `frustum` / `is_outside`：对点、AABB、bounding sphere 的视锥剔除；平面法线朝内。
- `primitives`：`unit_quad`、`unit_cube`、box、slab、cylinder、cone。
- 球体生成：UV 球（经纬网格）、icosphere（二十面体细分）、quad sphere（球化立方体）。
- 测试与基准：Catch2 测试 target、Google Benchmark 基准 target。

## 功能缺口与优先级

### P0：基础正确性与通用能力

| 能力 | 现状 / 缺口 |
|---|---|
| 三角形求交尺度 | `ray` 已统一为单位方向，方向尺度问题解决；但 `ray_triangle` 的 determinant 仍使用绝对 `1e-7`，极小或极大三角形可能误判。建议按边长缩放 epsilon。 |
| 网格变换 | 缺 `transform(mesh, mat4)`；法线需要用逆转置矩阵变换，不能直接乘原矩阵。 |
| 法线工具 | 缺 `recompute_normals`（smooth/flat）、smoothing group、flip winding、切线生成。 |
| 顶点焊接 | 缺按位置/属性去重的 `weld_vertices`；UV 接缝、多面接缝和外部导入网格都需要它。 |
| 网格校验 | 缺 `validate()`；`ray_mesh`、`flatten` 遇到越界索引会静默跳过，容易掩盖数据错误。 |
| 网格访问器 | 缺按三角形访问的 `triangle(i)`、`for_each_triangle`、`operator==`/hash。 |

### P1：游戏引擎常用图元与查询

| 能力 | 现状 / 缺口 |
|---|---|
| 图元 | 缺 capsule（角色/物理）、torus、disk/circle、grid/细分平面、frustum primitive（截头锥网格）、hemisphere。`create_cone` 没有 normal/caps 选项，图元选项风格不统一。 |
| 解析求交 | 缺 ray-sphere、ray-cylinder、ray-cone、ray-capsule；目前只有 bounding sphere 的近似求交。 |
| 加速结构 | `ray_mesh` 是 O(n) 线性扫描；需要 BVH 或 AABB tree，否则大网格拾取不可用。 |
| 其他查询 | 缺 triangle-triangle、point-in-mesh、closest point on triangle/mesh。 |
| 包围体 | 已有 `frustum` 的逐平面剔除；仍缺 OBB、AABB/sphere/triangle 之间的重叠测试、closest point，以及 `compute_bounding_sphere` 的最小包围球改进。 |
| 网格属性 | 缺 tangent、顶点颜色、第二套 UV、submesh/material range。 |

### P2：建模、拓扑与工程化

| 能力 | 现状 / 缺口 |
|---|---|
| 建模操作 | 缺 extrude、lathe/revolution、sweep/loft、tube、CSG、convex hull、decimate/LOD、subdivision surface。 |
| 拓扑 | 缺 half-edge/adjacency、连通分量、边界检测、平滑组。 |
| 2D 多边形 | 若“多边形库”也包含 2D：缺三角化（ear clipping）、Sutherland-Hodgman 裁剪、面积/质心、point-in-polygon、offset。 |
| 工程化 | 缺 CMake install/export、CI、Doxygen 文档、示例、fuzz 测试。 |

## 球体生成算法

三种算法解决不同问题，统一通过 `create_sphere(radius, sphere_options{ .kind = ... })` 选择：

| 算法 | 拓扑 / 参数 | 优点 | 代价 | 典型用途 |
|---|---|---|---|---|
| UV 球 | 经纬网格；`rings`、`segments` | 实现简单、UV 直观、与旧 API 一致 | 极点三角形挤压，1 条接缝 | 低模、天空球、快速预览 |
| Icosphere | 二十面体 `20·4^L` 细分；`ico_subdivisions` | 三角形近似均匀、无极点、适合位移/物理 | 没有自然 UV，需要球面 UV 和接缝拆分 | 行星、碰撞、程序化地形 |
| Quad sphere | 球化立方体；`quad_resolution` | 均匀性较好、6 面参数化、适合 LOD/cubemap | 6 个面接缝，三角形有对角线方向 | 行星 LOD、cubemap 风格、纹理分块 |

约定：

- 球心在原点，半径由参数指定；非法半径（`<= 0`、NaN、Inf）返回空网格。
- 三角形从外部看是逆时针，smooth 法线沿半径向外，flat 模式为每个三角形复制顶点并写入面法线。
- UV 球使用等距圆柱 UV，接缝列复制；icosphere 的球面 UV 按三角形做最短弧展开，接缝处复制顶点；quad sphere 每个立方体面使用独立 `[0,1]²` UV，面间复制顶点。
- 不继续增加近似变体（如 octasphere、Fibonacci 球）；没有明确用例前不扩大维护和测试面。Fibonacci 分布更适合独立的点集 API。

### 数量公式

| kind | 三角形数 | 唯一位置顶点数（焊接后） |
|---|---|---|
| uv | `2·segments·(rings-1)` | `2+(rings-1)·segments` |
| ico | `20·4^L` | `10·4^L+2` |
| quad | `12·n²` | `6·n²+2` |

## 已知问题

- `ray_triangle` 的 determinant 仍用绝对 epsilon，极小三角形可能漏检；`ray` 的方向尺度问题已由单位化解决。
- `compute_bounding_sphere` 不是最小包围球，仅适合粗略剔除。
- `mesh_data` 没有校验，坏索引可能被静默忽略。
- 图元选项不一致：cylinder 有 `caps`/`normal`，cone 固定 caps 且没有 normal 选项。
- `ray_mesh` 没有加速结构。
- `is_outside` 的 AABB/sphere 重载是逐平面保守测试：对象完全位于视锥外但靠近边/角、未完全越过任一平面时可能返回 false；点测试是精确的。
- UV 球极点附近的等距圆柱 UV 仍然有拉伸；这是映射本身的限制。

## 非目标 / 待确认

- 本路线图假设“几何库”主要服务于 3D 三角形网格。若还需要“球面多边形”本身（大圆弧边、球面面积、球面包含判定），应另设 `spherical_polygon` API。
- 2D 多边形操作是否纳入本库，需要根据实际使用场景确认。
- 是否引入 BVH、half-edge、CSG 等较重的数据结构，取决于引擎侧的实际需求。
