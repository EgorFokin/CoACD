#include "./process.h"
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

        random_engine.seed(params.seed);


        Model pmesh = mesh, pCH;
        Plane bestplane;
        pmesh.ComputeAPX(pCH, params.apx_mode, true);
        vector<Plane> planes, best_path;

        // MCTS for cutting plane
        Node *node = new Node(params);
        State state(params, pmesh);
        node->set_state(state);
        Node *best_next_node = MonteCarloTreeSearch(params, node, best_path);
        if (best_next_node == NULL)
        {

            parts.push_back(pCH);
            pmeshs.push_back(pmesh);
            free_tree(node, 0);
        }
        else
        {
            bestplane = best_next_node->state->current_value.first;
            TernaryMCTS(pmesh, params, bestplane, best_path, best_next_node->quality_value); // using Rv to Ternary refine
            free_tree(node, 0);

        }
        

        return bestplane;
    }

}