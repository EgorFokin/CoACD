#include "./process.h"
#include "./mod.h"
#include "mcts.h"
#include "config.h"
#include "bvh.h"

#include <iostream>
#include <cmath>


namespace coacd
{
    vector<pair<Node *,double>> best_children(Node *node, bool is_exploration, double initial_cost, int num_childen)
    {
        double best_score_max = INF;
        vector<pair<Node *,double>> best_sub_nodes = vector<pair<Node *,double>>();

        vector<Node *> children = node->get_children();
        for (int i = 0; i < (int)children.size(); i++)
        {
            double C;
            Node *sub_node = children[i];
            if (is_exploration)
                C = initial_cost / sqrt(2.0);
            else
                C = 0.0;

            double left = sub_node->get_quality_value();
            double right = 2.0 * log(node->get_visit_times()) / sub_node->get_visit_times();
            double score = left - C * sqrt(right);

            if (score < best_score_max)
            {
                
                best_sub_nodes.push_back(make_pair(sub_node, score));
                if (best_sub_nodes.size() > num_childen)
                {
                    double max_score = -INF;
                    int max_idx = 0;
                    for (int j = 0; j < (int)best_sub_nodes.size(); j++)
                    {
                        if (best_sub_nodes[j].second > max_score)
                        {
                            max_score = best_sub_nodes[j].second;
                            max_idx = j;
                        }
                    }
                    best_sub_nodes.erase(best_sub_nodes.begin() + max_idx);

                    max_score = -INF;
                    for (int j = 0; j < (int)best_sub_nodes.size(); j++)
                    {
                        if (best_sub_nodes[j].second > max_score)
                        {
                            max_score = best_sub_nodes[j].second;
                        }
                    }
                    best_score_max = max_score;
                    if (best_score_max == -INF) //empty vector
                    {
                        best_score_max = INF;
                    }

                }
            }
        }

        return best_sub_nodes;
    }



    vector<pair<Plane,double>> BestCuttingPlanes(Model &mesh, Params &params, int num_planes){
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

        vector<Plane>  best_path;
        vector<pair<Plane,double>> planes;

        // MCTS for cutting plane
        Node *node = new Node(params);
        State state(params, pmesh);
        node->set_state(state);
        Node *best_next_node = MonteCarloTreeSearch(params, node, best_path);

        vector<pair<Node *,double>> best_nodes = best_children(node, false, 0.1, num_planes);
        for (int i = 0; i < (int)best_nodes.size(); i++)   
        {
            Plane next_best_plane = best_nodes[i].first->state->current_value.first;

            TernaryMCTS(pmesh, params, next_best_plane, best_path, best_nodes[i].first->quality_value); // using Rv to Ternary refine
            planes.push_back(make_pair(next_best_plane, best_nodes[i].first->quality_value));
        }
        free_tree(node, 0);
        
        return planes;

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