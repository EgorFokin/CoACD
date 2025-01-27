#pragma once

#include <iostream>
#include <string>
#include <random>
#include <fstream>
#include <vector>
#include <math.h>
#include <limits>
#include <typeinfo>
#include <algorithm>
#include <assert.h>
#include <regex>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "./io.h"
#include "clip.h"
#include "config.h"
#include "model_obj.h"
#include "cost.h"

namespace coacd
{
    struct MeshScore{
        int hulls_num;
        double avg_concavity;
    };

    vector<pair<Plane,double>> BestCuttingPlanes(Model &mesh, Params &params, int num_planes = 1);

    MeshScore ComputeScore(Model &mesh, Params &params);


}