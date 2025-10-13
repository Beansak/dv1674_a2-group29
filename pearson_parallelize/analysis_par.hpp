/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// THIS IS FILE FOR PARRALLELIZED VERSION

#include "vector.hpp"
#include <vector>

#if !defined(ANALYSIS_HPP)
#define ANALYSIS_HPP

namespace Analysis
{
    std::vector<double> correlation_coefficients(std::vector<Vector> datasets, int nr_of_threads);
    double pearson(Vector vec1, Vector vec2);

    struct ThreadData
    {
        int thread_id;
        int n;
        std::vector<Vector> *datasets;
        std::vector<double> *means;
        std::vector<double> *result;

        int startpoint;
        int endpoint;
    };

};

#endif
