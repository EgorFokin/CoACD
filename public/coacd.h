#pragma once
#include <array>
#include <string>
#include <string_view>
#include <vector>

struct CoACD_Plane
{
  double a, b, c, d, score;
};

struct CoACD_MeshScore
{
  int hulls_num;
  double avg_concavity;
};

namespace coacd
{

#if _WIN32
#define COACD_API __declspec(dllexport)
#else
#define COACD_API
#endif

  struct Mesh
  {
    std::vector<std::array<double, 3>> vertices;
    std::vector<std::array<int, 3>> indices;
  };

  std::vector<Mesh> CoACD(Mesh const &input, double threshold = 0.05,
                          int max_convex_hull = -1, std::string preprocess = "auto",
                          int prep_resolution = 50, int sample_resolution = 2000,
                          int mcts_nodes = 20, int mcts_iteration = 150,
                          int mcts_max_depth = 3, bool pca = false,
                          bool merge = true, std::string apx_mode = "ch", unsigned int seed = 0);
  void set_log_level(std::string_view level);

  CoACD_Plane run_best_cutting_plane(Mesh const &input, double threshold = 0.05,
                                     int max_convex_hull = -1, std::string preprocess = "auto",
                                     int prep_resolution = 50, int sample_resolution = 2000,
                                     int mcts_nodes = 20, int mcts_iteration = 150,
                                     int mcts_max_depth = 3, bool pca = false,
                                     bool merge = true, std::string apx_mode = "ch", unsigned int seed = 0);

} // namespace coacd

extern "C"
{

  struct CoACD_Mesh
  {
    double *vertices_ptr;
    uint64_t vertices_count;
    int *triangles_ptr;
    uint64_t triangles_count;
  };

  struct CoACD_MeshArray
  {
    CoACD_Mesh *meshes_ptr;
    uint64_t meshes_count;
  };

  struct CoACD_PlaneArray
  {
    CoACD_Plane *planes_ptr;
    uint64_t planes_count;
  };

  struct CoACD_ConvexHull
  {
    CoACD_Mesh mesh;
    double concavity;
  };

  struct Normalization
  {
    CoACD_Mesh mesh;
    double x_min, x_max, y_min, y_max, z_min, z_max;
  };

  void COACD_API
  CoACD_freeMeshArray(CoACD_MeshArray arr);

  constexpr int preprocess_auto = 0;
  constexpr int preprocess_on = 1;
  constexpr int preprocess_off = 2;

  constexpr int apx_ch = 0;
  constexpr int apx_box = 1;

  CoACD_MeshArray COACD_API CoACD_run(CoACD_Mesh const &input, double threshold,
                                      int max_convex_hull, int preprocess_mode,
                                      int prep_resolution, int sample_resolution,
                                      int mcts_nodes, int mcts_iteration,
                                      int mcts_max_depth, bool pca, bool merge,
                                      bool decimate, int max_ch_vertex,
                                      bool extrude, double extrude_margin,
                                      int apx_mode, unsigned int seed);

  void COACD_API CoACD_setLogLevel(char const *level);

  CoACD_PlaneArray COACD_API CoACD_bestCuttingPlanes(CoACD_Mesh const &input, double threshold,
                                                     int max_convex_hull, int preprocess_mode,
                                                     int prep_resolution, int sample_resolution,
                                                     int mcts_nodes, int mcts_iteration,
                                                     int mcts_max_depth, bool pca, bool merge,
                                                     bool decimate, int max_ch_vertex,
                                                     bool extrude, double extrude_margin,
                                                     int apx_mode, unsigned int seed, int num_planes);

  void COACD_API CoACD_freePlaneArray(CoACD_PlaneArray arr);

  void COACD_API CoACD_freeMesh(CoACD_Mesh mesh);

  CoACD_MeshScore COACD_API CoACD_meshScore(CoACD_Mesh const &input, double threshold,
                                            int max_convex_hull, int preprocess_mode,
                                            int prep_resolution, int sample_resolution,
                                            int mcts_nodes, int mcts_iteration,
                                            int mcts_max_depth, bool pca, bool merge,
                                            bool decimate, int max_ch_vertex,
                                            bool extrude, double extrude_margin,
                                            int apx_mode, unsigned int seed);

  Normalization COACD_API CoACD_normalize(CoACD_Mesh const &input, bool pca);

  CoACD_MeshArray COACD_API CoACD_clip(CoACD_Mesh const &input, CoACD_Plane const &plane);

  CoACD_ConvexHull COACD_API CoACD_compute_convex_hull(CoACD_Mesh const &input);

  CoACD_MeshArray COACD_API CoACD_merge(CoACD_Mesh const &mesh, CoACD_MeshArray pmeshs, CoACD_MeshArray parts);
}
