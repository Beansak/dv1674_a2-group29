/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// THIS IS FILE FOR PARRALLELIZED VERSION

#include "vector.hpp"
#include <vector>
#include <pthread.h>

#if !defined(ANALYSIS_HPP)
#define ANALYSIS_HPP

namespace Analysis
{
    std::vector<double> correlation_coefficients(std::vector<Vector> datasets, int nr_of_threads);
    double pearson(Vector vec1, Vector vec2);

    struct ThreadData
    {
        int thread_id;
        int nr_of_threads;
        int n;
        std::vector<Vector> *datasets;
        std::vector<Vector> *normalized;
        std::vector<double> *means;
        std::vector<double> *result;

        int precompute_start, precompute_end;
        int correlation_start, correlation_end;

        pthread_barrier_t *barrier;
    };

};

#endif
