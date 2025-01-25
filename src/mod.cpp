#include "./process.h"
#include "./mod.h"
#include "mcts.h"
#include "config.h"
#include "bvh.h"

#include <iostream>
#include <cmath>


namespace coacd
{

    Plane BestCuttingPlane(Model &mesh, Params &params){
        vector<Model> parts, pmeshs;
        clock_t start, end;
        start = clock();

        logger::info("# Points: {}", mesh.points.size());
        logger::info("# Triangles: {}", mesh.triangles.size());
        logger::info(" - Decomposition (MCTS)");

        size_t iter = 0;
        double cut_area;

        random_engine.seed(params.seed);

        Model pmesh = mesh, pCH;
        Plane bestplane;
        pmesh.ComputeAPX(pCH, params.apx_mode, true);
        double h = ComputeHCost(pmesh, pCH, params.rv_k, params.resolution, params.seed, 0.0001, false);

        vector<Plane> planes, best_path;

        // MCTS for cutting plane
        Node *node = new Node(params);
        State state(params, pmesh);
        node->set_state(state);
        Node *best_next_node = MonteCarloTreeSearch(params, node, best_path);

        bestplane = best_next_node->state->current_value.first;
        TernaryMCTS(pmesh, params, bestplane, best_path, best_next_node->quality_value); // using Rv to Ternary refine
        free_tree(node, 0);


        return bestplane;
    }

    MeshScore ComputeScore(Model &mesh, Params &params){
        vector<Model> parts = Compute(mesh, params);

        MeshScore score;
        score.hulls_num = parts.size();
        double avg_concavity = 0;
        for (int i = 0; i < (int)parts.size(); i++)
        {
            Model ch;
            parts[i].ComputeAPX(ch, params.apx_mode, true);
            avg_concavity += ComputeHCost(parts[i], ch, params.rv_k, params.resolution, params.seed, 0.0001, false);
        }
        avg_concavity /= parts.size();
        score.avg_concavity = avg_concavity;
        return score;
    }

}