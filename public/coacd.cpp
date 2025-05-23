#include "coacd.h"
#include "../src/logger.h"
#include "../src/preprocess.h"
#include "../src/process.h"
#include "../src/mod.h"

namespace coacd
{
  void RecoverParts(vector<Model> &meshes, vector<double> bbox,
                    array<array<double, 3>, 3> rot)
  {
    for (int i = 0; i < (int)meshes.size(); i++)
    {
      meshes[i].Recover(bbox);
      meshes[i].RevertPCA(rot);
    }
  }

  std::vector<Mesh> CoACD(Mesh const &input, double threshold,
                          int max_convex_hull, std::string preprocess_mode,
                          int prep_resolution, int sample_resolution,
                          int mcts_nodes, int mcts_iteration, int mcts_max_depth,
                          bool pca, bool merge, bool decimate, int max_ch_vertex,
                          bool extrude, double extrude_margin,
                          std::string apx_mode, unsigned int seed)
  {

    logger::info("threshold               {}", threshold);
    logger::info("max # convex hull       {}", max_convex_hull);
    logger::info("preprocess mode         {}", preprocess_mode);
    logger::info("preprocess resolution   {}", prep_resolution);
    logger::info("pca                     {}", pca);
    logger::info("mcts max depth          {}", mcts_max_depth);
    logger::info("mcts nodes              {}", mcts_nodes);
    logger::info("mcts iterations         {}", mcts_iteration);
    logger::info("merge                   {}", merge);
    logger::info("decimate                {}", decimate);
    logger::info("max_ch_vertex           {}", max_ch_vertex);
    logger::info("extrude                 {}", extrude);
    logger::info("extrude margin          {}", extrude_margin);
    logger::info("approximate mode        {}", apx_mode);
    logger::info("seed                    {}", seed);

    if (threshold < 0.01)
    {
      throw std::runtime_error("CoACD threshold < 0.01 (should be 0.01-1).");
    }
    else if (threshold > 1)
    {
      throw std::runtime_error("CoACD threshold > 1 (should be 0.01-1).");
    }

    if (prep_resolution > 1000)
    {
      throw std::runtime_error("CoACD prep resolution > 1000, this is probably a "
                               "bug (should be 30-100).");
    }
    else if (prep_resolution < 5)
    {
      throw std::runtime_error("CoACD prep resolution < 5, this is probably a "
                               "bug (should be 20-100).");
    }

    Params params;
    params.input_model = "";
    params.output_name = "";
    params.threshold = threshold;
    params.max_convex_hull = max_convex_hull;
    params.preprocess_mode = preprocess_mode;
    params.prep_resolution = prep_resolution;
    params.resolution = sample_resolution;
    params.mcts_nodes = mcts_nodes;
    params.mcts_iteration = mcts_iteration;
    params.mcts_max_depth = mcts_max_depth;
    params.pca = pca;
    params.merge = merge;
    params.decimate = decimate;
    params.max_ch_vertex = max_ch_vertex;
    params.extrude = extrude;
    params.extrude_margin = extrude_margin;
    params.apx_mode = apx_mode;
    params.seed = seed;

    Model m;
    m.Load(input.vertices, input.indices);
    vector<double> bbox = m.Normalize();
    array<array<double, 3>, 3> rot{
        {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};

    if (params.preprocess_mode == std::string("auto"))
    {
      bool is_manifold = IsManifold(m);
      logger::info("Mesh Manifoldness: {}", is_manifold);
      if (!is_manifold)
        ManifoldPreprocess(params, m);
    }
    else if (params.preprocess_mode == std::string("on"))
    {
      ManifoldPreprocess(params, m);
    }

    if (pca)
    {
      rot = m.PCA();
    }

    vector<Model> parts = Compute(m, params);
    RecoverParts(parts, bbox, rot);

    std::vector<Mesh> result;
    for (auto &p : parts)
    {
      result.push_back(Mesh{.vertices = p.points, .indices = p.triangles});
    }
    return result;
  }

  void set_log_level(std::string_view level)
  {
    if (level == "off")
    {
      logger::get()->set_level(spdlog::level::off);
    }
    else if (level == "debug")
    {
      logger::get()->set_level(spdlog::level::debug);
    }
    else if (level == "info")
    {
      logger::get()->set_level(spdlog::level::info);
    }
    else if (level == "warn" || level == "warning")
    {
      logger::get()->set_level(spdlog::level::warn);
    }
    else if (level == "error" || level == "err")
    {
      logger::get()->set_level(spdlog::level::err);
    }
    else if (level == "critical")
    {
      logger::get()->set_level(spdlog::level::critical);
    }
    else
    {
      throw std::runtime_error("invalid log level " + std::string(level));
    }
  }

  vector<CoACD_Plane> run_best_cutting_planes(Mesh const &input, double threshold,
                                              int max_convex_hull, std::string preprocess_mode,
                                              int prep_resolution, int sample_resolution,
                                              int mcts_nodes, int mcts_iteration, int mcts_max_depth,
                                              bool pca, bool merge, bool decimate, int max_ch_vertex,
                                              bool extrude, double extrude_margin,
                                              std::string apx_mode, unsigned int seed, int num_planes)
  {

    if (threshold < 0.01)
    {
      throw std::runtime_error("CoACD threshold < 0.01 (should be 0.01-1).");
    }
    else if (threshold > 1)
    {
      throw std::runtime_error("CoACD threshold > 1 (should be 0.01-1).");
    }

    if (prep_resolution > 1000)
    {
      throw std::runtime_error("CoACD prep resolution > 1000, this is probably a "
                               "bug (should be 30-100).");
    }
    else if (prep_resolution < 5)
    {
      throw std::runtime_error("CoACD prep resolution < 5, this is probably a "
                               "bug (should be 20-100).");
    }

    Params params;
    params.input_model = "";
    params.output_name = "";
    params.threshold = threshold;
    params.max_convex_hull = max_convex_hull;
    params.preprocess_mode = preprocess_mode;
    params.prep_resolution = prep_resolution;
    params.resolution = sample_resolution;
    params.mcts_nodes = mcts_nodes;
    params.mcts_iteration = mcts_iteration;
    params.mcts_max_depth = mcts_max_depth;
    params.pca = pca;
    params.merge = merge;
    params.decimate = decimate;
    params.max_ch_vertex = max_ch_vertex;
    params.extrude = extrude;
    params.extrude_margin = extrude_margin;
    params.apx_mode = apx_mode;
    params.seed = seed;

    Model m;
    m.Load(input.vertices, input.indices);
    vector<double> bbox = m.Normalize();
    array<array<double, 3>, 3> rot{
        {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};

    if (params.preprocess_mode == std::string("auto"))
    {
      bool is_manifold = IsManifold(m);
      logger::info("Mesh Manifoldness: {}", is_manifold);
      if (!is_manifold)
        ManifoldPreprocess(params, m);
    }
    else if (params.preprocess_mode == std::string("on"))
    {
      ManifoldPreprocess(params, m);
    }

    if (pca)
    {
      rot = m.PCA();
    }

    vector<pair<coacd::Plane, double>> bestplanes = BestCuttingPlanes(m, params, num_planes);

    vector<CoACD_Plane> planes;
    for (auto &p : bestplanes)
    {
      planes.push_back(CoACD_Plane{.a = p.first.a, .b = p.first.b, .c = p.first.c, .d = p.first.d, .score = p.second});
    }

    return planes;
  }

  CoACD_MeshScore run_mesh_score(Mesh const &input, double threshold,
                                 int max_convex_hull, std::string preprocess_mode,
                                 int prep_resolution, int sample_resolution,
                                 int mcts_nodes, int mcts_iteration, int mcts_max_depth,
                                 bool pca, bool merge, bool decimate, int max_ch_vertex,
                                 bool extrude, double extrude_margin,
                                 std::string apx_mode, unsigned int seed)
  {

    if (threshold < 0.01)
    {
      throw std::runtime_error("CoACD threshold < 0.01 (should be 0.01-1).");
    }
    else if (threshold > 1)
    {
      throw std::runtime_error("CoACD threshold > 1 (should be 0.01-1).");
    }

    if (prep_resolution > 1000)
    {
      throw std::runtime_error("CoACD prep resolution > 1000, this is probably a "
                               "bug (should be 30-100).");
    }
    else if (prep_resolution < 5)
    {
      throw std::runtime_error("CoACD prep resolution < 5, this is probably a "
                               "bug (should be 20-100).");
    }

    Params params;
    params.input_model = "";
    params.output_name = "";
    params.threshold = threshold;
    params.max_convex_hull = max_convex_hull;
    params.preprocess_mode = preprocess_mode;
    params.prep_resolution = prep_resolution;
    params.resolution = sample_resolution;
    params.mcts_nodes = mcts_nodes;
    params.mcts_iteration = mcts_iteration;
    params.mcts_max_depth = mcts_max_depth;
    params.pca = pca;
    params.merge = merge;
    params.decimate = decimate;
    params.max_ch_vertex = max_ch_vertex;
    params.extrude = extrude;
    params.extrude_margin = extrude_margin;
    params.apx_mode = apx_mode;
    params.seed = seed;

    Model m;
    m.Load(input.vertices, input.indices);
    vector<double> bbox = m.Normalize();
    array<array<double, 3>, 3> rot{
        {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};

    if (params.preprocess_mode == std::string("auto"))
    {
      bool is_manifold = IsManifold(m);
      logger::info("Mesh Manifoldness: {}", is_manifold);
      if (!is_manifold)
        ManifoldPreprocess(params, m);
    }
    else if (params.preprocess_mode == std::string("on"))
    {
      ManifoldPreprocess(params, m);
    }

    if (pca)
    {
      rot = m.PCA();
    }

    coacd::MeshScore score = ComputeScore(m, params);

    CoACD_MeshScore mesh_score;
    mesh_score.hulls_num = score.hulls_num;
    mesh_score.avg_concavity = score.avg_concavity;

    return mesh_score;
  }

} // namespace coacd

extern "C"
{
  void CoACD_freeMeshArray(CoACD_MeshArray arr)
  {
    for (uint64_t i = 0; i < arr.meshes_count; ++i)
    {
      delete[] arr.meshes_ptr[i].vertices_ptr;
      arr.meshes_ptr[i].vertices_ptr = nullptr;
      arr.meshes_ptr[i].vertices_count = 0;
      delete[] arr.meshes_ptr[i].triangles_ptr;
      arr.meshes_ptr[i].triangles_ptr = nullptr;
      arr.meshes_ptr[i].triangles_count = 0;
    }
    arr.meshes_count = 0;
    arr.meshes_ptr = nullptr;
    delete[] arr.meshes_ptr;
  }

  CoACD_MeshArray CoACD_run(CoACD_Mesh const &input, double threshold,
                            int max_convex_hull, int preprocess_mode,
                            int prep_resolution, int sample_resolution,
                            int mcts_nodes, int mcts_iteration,
                            int mcts_max_depth, bool pca, bool merge,
                            bool decimate, int max_ch_vertex,
                            bool extrude, double extrude_margin,
                            int apx_mode, unsigned int seed)
  {
    coacd::Mesh mesh;
    for (uint64_t i = 0; i < input.vertices_count; ++i)
    {
      mesh.vertices.push_back({input.vertices_ptr[3 * i],
                               input.vertices_ptr[3 * i + 1],
                               input.vertices_ptr[3 * i + 2]});
    }
    for (uint64_t i = 0; i < input.triangles_count; ++i)
    {
      mesh.indices.push_back({input.triangles_ptr[3 * i],
                              input.triangles_ptr[3 * i + 1],
                              input.triangles_ptr[3 * i + 2]});
    }

    std::string pm, apx;
    if (preprocess_mode == preprocess_on)
    {
      pm = "on";
    }
    else if (preprocess_mode == preprocess_off)
    {
      pm = "off";
    }
    else
    {
      pm = "auto";
    }

    if (apx_mode == apx_ch)
    {
      apx = "ch";
    }
    else if (apx_mode == apx_box)
    {
      apx = "box";
    }
    else
    {
      throw std::runtime_error("invalid approximation mode " + std::to_string(apx_mode));
    }

    auto meshes = coacd::CoACD(mesh, threshold, max_convex_hull, pm,
                               prep_resolution, sample_resolution, mcts_nodes,
                               mcts_iteration, mcts_max_depth, pca, merge, decimate, max_ch_vertex,
                               extrude, extrude_margin, apx, seed);

    CoACD_MeshArray arr;
    arr.meshes_ptr = new CoACD_Mesh[meshes.size()];
    arr.meshes_count = meshes.size();

    for (size_t i = 0; i < meshes.size(); ++i)
    {
      arr.meshes_ptr[i].vertices_ptr = new double[meshes[i].vertices.size() * 3];
      arr.meshes_ptr[i].vertices_count = meshes[i].vertices.size();
      for (size_t j = 0; j < meshes[i].vertices.size(); ++j)
      {
        arr.meshes_ptr[i].vertices_ptr[3 * j] = meshes[i].vertices[j][0];
        arr.meshes_ptr[i].vertices_ptr[3 * j + 1] = meshes[i].vertices[j][1];
        arr.meshes_ptr[i].vertices_ptr[3 * j + 2] = meshes[i].vertices[j][2];
      }
      arr.meshes_ptr[i].triangles_ptr = new int[meshes[i].indices.size() * 3];
      arr.meshes_ptr[i].triangles_count = meshes[i].indices.size();
      for (size_t j = 0; j < meshes[i].indices.size(); ++j)
      {
        arr.meshes_ptr[i].triangles_ptr[3 * j] = meshes[i].indices[j][0];
        arr.meshes_ptr[i].triangles_ptr[3 * j + 1] = meshes[i].indices[j][1];
        arr.meshes_ptr[i].triangles_ptr[3 * j + 2] = meshes[i].indices[j][2];
      }
    }
    return arr;
  }

  void CoACD_setLogLevel(char const *level)
  {
    coacd::set_log_level(std::string_view(level));
  }

  void CoACD_freeMesh(CoACD_Mesh mesh)
  {
    delete[] mesh.vertices_ptr;
    mesh.vertices_ptr = nullptr;
    mesh.vertices_count = 0;
    delete[] mesh.triangles_ptr;
    mesh.triangles_ptr = nullptr;
    mesh.triangles_count = 0;
  }

  CoACD_PlaneArray CoACD_bestCuttingPlanes(CoACD_Mesh const &input, double threshold,
                                           int max_convex_hull, int preprocess_mode,
                                           int prep_resolution, int sample_resolution,
                                           int mcts_nodes, int mcts_iteration,
                                           int mcts_max_depth, bool pca, bool merge,
                                           bool decimate, int max_ch_vertex,
                                           bool extrude, double extrude_margin,
                                           int apx_mode, unsigned int seed, int num_planes)
  {
    coacd::Mesh mesh;
    for (uint64_t i = 0; i < input.vertices_count; ++i)
    {
      mesh.vertices.push_back({input.vertices_ptr[3 * i],
                               input.vertices_ptr[3 * i + 1],
                               input.vertices_ptr[3 * i + 2]});
    }
    for (uint64_t i = 0; i < input.triangles_count; ++i)
    {
      mesh.indices.push_back({input.triangles_ptr[3 * i],
                              input.triangles_ptr[3 * i + 1],
                              input.triangles_ptr[3 * i + 2]});
    }

    std::string pm, apx;
    if (preprocess_mode == preprocess_on)
    {
      pm = "on";
    }
    else if (preprocess_mode == preprocess_off)
    {
      pm = "off";
    }
    else
    {
      pm = "auto";
    }

    if (apx_mode == apx_ch)
    {
      apx = "ch";
    }
    else if (apx_mode == apx_box)
    {
      apx = "box";
    }
    else
    {
      throw std::runtime_error("invalid approximation mode " + std::to_string(apx_mode));
    }
    vector<CoACD_Plane> bestplanes;
    try
    {
      bestplanes = coacd::run_best_cutting_planes(mesh, threshold, max_convex_hull, pm,
                                                  prep_resolution, sample_resolution, mcts_nodes,
                                                  mcts_iteration, mcts_max_depth, pca, merge, decimate, max_ch_vertex,
                                                  extrude, extrude_margin, apx, seed, num_planes);
    }
    catch (const std::exception &e)
    {
      CoACD_PlaneArray arr;
      arr.planes_count = 0;
      arr.planes_ptr = nullptr;
      return arr;
    };

    CoACD_PlaneArray planes;
    planes.planes_ptr = new CoACD_Plane[bestplanes.size()];
    planes.planes_count = bestplanes.size();

    for (size_t i = 0; i < bestplanes.size(); ++i)
    {
      planes.planes_ptr[i].a = bestplanes[i].a;
      planes.planes_ptr[i].b = bestplanes[i].b;
      planes.planes_ptr[i].c = bestplanes[i].c;
      planes.planes_ptr[i].d = bestplanes[i].d;
      planes.planes_ptr[i].score = bestplanes[i].score;
    }

    return planes;
  }

  void CoACD_freePlaneArray(CoACD_PlaneArray arr)
  {
    delete[] arr.planes_ptr;
    arr.planes_ptr = nullptr;
    arr.planes_count = 0;
  }

  CoACD_MeshScore CoACD_meshScore(CoACD_Mesh const &input, double threshold,
                                  int max_convex_hull, int preprocess_mode,
                                  int prep_resolution, int sample_resolution,
                                  int mcts_nodes, int mcts_iteration,
                                  int mcts_max_depth, bool pca, bool merge,
                                  bool decimate, int max_ch_vertex,
                                  bool extrude, double extrude_margin,
                                  int apx_mode, unsigned int seed)
  {
    coacd::Mesh mesh;
    for (uint64_t i = 0; i < input.vertices_count; ++i)
    {
      mesh.vertices.push_back({input.vertices_ptr[3 * i],
                               input.vertices_ptr[3 * i + 1],
                               input.vertices_ptr[3 * i + 2]});
    }
    for (uint64_t i = 0; i < input.triangles_count; ++i)
    {
      mesh.indices.push_back({input.triangles_ptr[3 * i],
                              input.triangles_ptr[3 * i + 1],
                              input.triangles_ptr[3 * i + 2]});
    }

    std::string pm, apx;
    if (preprocess_mode == preprocess_on)
    {
      pm = "on";
    }
    else if (preprocess_mode == preprocess_off)
    {
      pm = "off";
    }
    else
    {
      pm = "auto";
    }

    if (apx_mode == apx_ch)
    {
      apx = "ch";
    }
    else if (apx_mode == apx_box)
    {
      apx = "box";
    }
    else
    {
      throw std::runtime_error("invalid approximation mode " + std::to_string(apx_mode));
    }

    CoACD_MeshScore meshScore = coacd::run_mesh_score(mesh, threshold, max_convex_hull, pm,
                                                      prep_resolution, sample_resolution, mcts_nodes,
                                                      mcts_iteration, mcts_max_depth, pca, merge, decimate, max_ch_vertex,
                                                      extrude, extrude_margin, apx, seed);

    return meshScore;
  }

  Normalization CoACD_normalize(CoACD_Mesh const &input, bool pca)
  {
    coacd::Mesh mesh;
    for (uint64_t i = 0; i < input.vertices_count; ++i)
    {
      mesh.vertices.push_back({input.vertices_ptr[3 * i],
                               input.vertices_ptr[3 * i + 1],
                               input.vertices_ptr[3 * i + 2]});
    }
    for (uint64_t i = 0; i < input.triangles_count; ++i)
    {
      mesh.indices.push_back({input.triangles_ptr[3 * i],
                              input.triangles_ptr[3 * i + 1],
                              input.triangles_ptr[3 * i + 2]});
    }

    array<array<double, 3>, 3> rot{
        {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}};

    coacd::Model m;
    m.Load(mesh.vertices, mesh.indices);
    vector<double> bbox = m.Normalize();

    if (pca)
    {
      rot = m.PCA();
    }

    mesh.vertices = m.points;

    CoACD_Mesh result;
    result.vertices_ptr = new double[mesh.vertices.size() * 3];
    result.vertices_count = mesh.vertices.size();
    for (size_t j = 0; j < mesh.vertices.size(); ++j)
    {
      result.vertices_ptr[3 * j] = mesh.vertices[j][0];
      result.vertices_ptr[3 * j + 1] = mesh.vertices[j][1];
      result.vertices_ptr[3 * j + 2] = mesh.vertices[j][2];
    }
    result.triangles_ptr = new int[mesh.indices.size() * 3];
    result.triangles_count = mesh.indices.size();
    for (size_t j = 0; j < mesh.indices.size(); ++j)
    {
      result.triangles_ptr[3 * j] = mesh.indices[j][0];
      result.triangles_ptr[3 * j + 1] = mesh.indices[j][1];
      result.triangles_ptr[3 * j + 2] = mesh.indices[j][2];
    }

    Normalization norm;
    norm.mesh = result;
    norm.x_min = bbox[0];
    norm.x_max = bbox[1];
    norm.y_min = bbox[2];
    norm.y_max = bbox[3];
    norm.z_min = bbox[4];
    norm.z_max = bbox[5];
    return norm;
  }

  CoACD_MeshArray CoACD_clip(CoACD_Mesh const &input, CoACD_Plane const &plane)
  {
    coacd::Mesh mesh;
    for (uint64_t i = 0; i < input.vertices_count; ++i)
    {
      mesh.vertices.push_back({input.vertices_ptr[3 * i],
                               input.vertices_ptr[3 * i + 1],
                               input.vertices_ptr[3 * i + 2]});
    }
    for (uint64_t i = 0; i < input.triangles_count; ++i)
    {
      mesh.indices.push_back({input.triangles_ptr[3 * i],
                              input.triangles_ptr[3 * i + 1],
                              input.triangles_ptr[3 * i + 2]});
    }

    coacd::Plane p;
    p.a = plane.a;
    p.b = plane.b;
    p.c = plane.c;
    p.d = plane.d;

    coacd::Model m, pos, neg;
    m.Load(mesh.vertices, mesh.indices);
    double cut_area;
    bool flag;
    try
    {
      flag = coacd::Clip(m, pos, neg, p, cut_area);
    }
    catch (const std::exception &e)
    {
      return CoACD_MeshArray();
    }

    CoACD_MeshArray arr;

    if (!flag)
    {
      arr.meshes_count = 0;
      arr.meshes_ptr = nullptr;
      return arr;
    }

    arr.meshes_ptr = new CoACD_Mesh[2];
    arr.meshes_count = 2;

    for (int i = 0; i < 2; i++)
    {
      coacd::Model *part = (i == 0) ? &pos : &neg;
      mesh.vertices = part->points;
      mesh.indices = part->triangles;

      arr.meshes_ptr[i].vertices_ptr = new double[mesh.vertices.size() * 3];
      arr.meshes_ptr[i].vertices_count = mesh.vertices.size();
      for (size_t j = 0; j < mesh.vertices.size(); ++j)
      {
        arr.meshes_ptr[i].vertices_ptr[3 * j] = mesh.vertices[j][0];
        arr.meshes_ptr[i].vertices_ptr[3 * j + 1] = mesh.vertices[j][1];
        arr.meshes_ptr[i].vertices_ptr[3 * j + 2] = mesh.vertices[j][2];
      }
      arr.meshes_ptr[i].triangles_ptr = new int[mesh.indices.size() * 3];
      arr.meshes_ptr[i].triangles_count = mesh.indices.size();
      for (size_t j = 0; j < mesh.indices.size(); ++j)
      {
        arr.meshes_ptr[i].triangles_ptr[3 * j] = mesh.indices[j][0];
        arr.meshes_ptr[i].triangles_ptr[3 * j + 1] = mesh.indices[j][1];
        arr.meshes_ptr[i].triangles_ptr[3 * j + 2] = mesh.indices[j][2];
      }
    }

    return arr;
  }

  CoACD_ConvexHull CoACD_compute_convex_hull(CoACD_Mesh const &input)
  {
    coacd::Mesh mesh;
    for (uint64_t i = 0; i < input.vertices_count; ++i)
    {
      mesh.vertices.push_back({input.vertices_ptr[3 * i],
                               input.vertices_ptr[3 * i + 1],
                               input.vertices_ptr[3 * i + 2]});
    }
    for (uint64_t i = 0; i < input.triangles_count; ++i)
    {
      mesh.indices.push_back({input.triangles_ptr[3 * i],
                              input.triangles_ptr[3 * i + 1],
                              input.triangles_ptr[3 * i + 2]});
    }

    coacd::Model m, chmesh;
    m.Load(mesh.vertices, mesh.indices);
    m.ComputeAPX(chmesh, "ch", true);

    double concavity = coacd::ComputeHCost(m, chmesh, 0.3, 2000, 0, 0.0001, false);

    CoACD_ConvexHull ch;
    ch.mesh.vertices_ptr = new double[chmesh.points.size() * 3];
    ch.mesh.vertices_count = chmesh.points.size();
    for (size_t j = 0; j < chmesh.points.size(); ++j)
    {
      ch.mesh.vertices_ptr[3 * j] = chmesh.points[j][0];
      ch.mesh.vertices_ptr[3 * j + 1] = chmesh.points[j][1];
      ch.mesh.vertices_ptr[3 * j + 2] = chmesh.points[j][2];
    }
    ch.mesh.triangles_ptr = new int[chmesh.triangles.size() * 3];
    ch.mesh.triangles_count = chmesh.triangles.size();
    for (size_t j = 0; j < chmesh.triangles.size(); ++j)
    {
      ch.mesh.triangles_ptr[3 * j] = chmesh.triangles[j][0];
      ch.mesh.triangles_ptr[3 * j + 1] = chmesh.triangles[j][1];
      ch.mesh.triangles_ptr[3 * j + 2] = chmesh.triangles[j][2];
    }
    ch.concavity = concavity;

    return ch;
  }

  CoACD_MeshArray CoACD_merge(CoACD_Mesh const &mesh, CoACD_MeshArray pmeshs, CoACD_MeshArray parts)
  {
    coacd::Mesh m;
    for (uint64_t i = 0; i < mesh.vertices_count; ++i)
    {
      m.vertices.push_back({mesh.vertices_ptr[3 * i],
                            mesh.vertices_ptr[3 * i + 1],
                            mesh.vertices_ptr[3 * i + 2]});
    }
    for (uint64_t i = 0; i < mesh.triangles_count; ++i)
    {
      m.indices.push_back({mesh.triangles_ptr[3 * i],
                           mesh.triangles_ptr[3 * i + 1],
                           mesh.triangles_ptr[3 * i + 2]});
    }

    coacd::Model model;
    model.Load(m.vertices, m.indices);

    vector<coacd::Model> pms;
    for (uint64_t i = 0; i < pmeshs.meshes_count; ++i)
    {
      coacd::Mesh pm;
      for (uint64_t j = 0; j < pmeshs.meshes_ptr[i].vertices_count; ++j)
      {
        pm.vertices.push_back({pmeshs.meshes_ptr[i].vertices_ptr[3 * j],
                               pmeshs.meshes_ptr[i].vertices_ptr[3 * j + 1],
                               pmeshs.meshes_ptr[i].vertices_ptr[3 * j + 2]});
      }
      for (uint64_t j = 0; j < pmeshs.meshes_ptr[i].triangles_count; ++j)
      {
        pm.indices.push_back({pmeshs.meshes_ptr[i].triangles_ptr[3 * j],
                              pmeshs.meshes_ptr[i].triangles_ptr[3 * j + 1],
                              pmeshs.meshes_ptr[i].triangles_ptr[3 * j + 2]});
      }
      coacd::Model pm_model;
      pm_model.Load(pm.vertices, pm.indices);
      pms.push_back(pm_model);
    }

    vector<coacd::Model> ps;
    for (uint64_t i = 0; i < parts.meshes_count; ++i)
    {
      coacd::Mesh part;
      for (uint64_t j = 0; j < parts.meshes_ptr[i].vertices_count; ++j)
      {
        part.vertices.push_back({parts.meshes_ptr[i].vertices_ptr[3 * j],
                                 parts.meshes_ptr[i].vertices_ptr[3 * j + 1],
                                 parts.meshes_ptr[i].vertices_ptr[3 * j + 2]});
      }
      for (uint64_t j = 0; j < parts.meshes_ptr[i].triangles_count; ++j)
      {
        part.indices.push_back({parts.meshes_ptr[i].triangles_ptr[3 * j],
                                parts.meshes_ptr[i].triangles_ptr[3 * j + 1],
                                parts.meshes_ptr[i].triangles_ptr[3 * j + 2]});
      }
      coacd::Model part_model;
      part_model.Load(part.vertices, part.indices);
      ps.push_back(part_model);
    }

    coacd::Params default_params = coacd::Params();
    coacd::MergeConvexHulls(model, pms, ps, default_params);

    CoACD_MeshArray arr;
    arr.meshes_ptr = new CoACD_Mesh[ps.size()];
    arr.meshes_count = ps.size();

    for (size_t i = 0; i < ps.size(); ++i)
    {
      arr.meshes_ptr[i].vertices_ptr = new double[ps[i].points.size() * 3];
      arr.meshes_ptr[i].vertices_count = ps[i].points.size();
      for (size_t j = 0; j < ps[i].points.size(); ++j)
      {
        arr.meshes_ptr[i].vertices_ptr[3 * j] = ps[i].points[j][0];
        arr.meshes_ptr[i].vertices_ptr[3 * j + 1] = ps[i].points[j][1];
        arr.meshes_ptr[i].vertices_ptr[3 * j + 2] = ps[i].points[j][2];
      }
      arr.meshes_ptr[i].triangles_ptr = new int[ps[i].triangles.size() * 3];
      arr.meshes_ptr[i].triangles_count = ps[i].triangles.size();
      for (size_t j = 0; j < ps[i].triangles.size(); ++j)
      {
        arr.meshes_ptr[i].triangles_ptr[3 * j] = ps[i].triangles[j][0];
        arr.meshes_ptr[i].triangles_ptr[3 * j + 1] = ps[i].triangles[j][1];
        arr.meshes_ptr[i].triangles_ptr[3 * j + 2] = ps[i].triangles[j][2];
      }
    }

    return arr;
  }
}
