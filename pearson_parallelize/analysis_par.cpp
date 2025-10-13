/*
Author: David Holmqvist daae19@student.bth.se
*/

// THIS IS FILE FOR PARRALLELIZED VERSION

#include "analysis_par.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>
#include <pthread.h>
#include <mutex>

namespace Analysis
{
    std::mutex result_mutex;

    void *thread_function(void *arg)
    {
        ThreadData *data = static_cast<ThreadData *>(arg);
        int thread_id = data->thread_id;
        int n = data->n;
        std::vector<Vector> *datasets = data->datasets;
        std::vector<double> *means = data->means;
        std::vector<double> *result = data->result;

        int startpoint = data->startpoint;
        int endpoint = data->endpoint;

        for (size_t i = startpoint; i < endpoint; i++)
        {
            means->at(i) = datasets->at(i).mean();
        }

        // precompute normalized vectors (mean-subtracted and divided by magnitude)
        std::vector<Vector> normalized;
        normalized.reserve(n);

        for (size_t i = 0; i < n; i++)
        {
            auto mm = datasets->at(i) - means->at(i);
            auto mag = mm.magnitude();
            normalized.push_back(mm / mag); // Store the fully normalized vector
        }

        for (size_t sample1 = startpoint; sample1 < endpoint - 1; sample1++)
        {
            for (size_t sample2 = sample1 + 1; sample2 < n; sample2++)
            {
                auto r = normalized[sample1].dot(normalized[sample2]);
                double clamped_r = std::max(std::min(r, 1.0), -1.0);

                // Synchronize access to the result vector
                std::lock_guard<std::mutex> lock(result_mutex);
                result->push_back(clamped_r);
            }
        }

        // Cast the argument back to the appropriate type
        // Perform thread-specific tasks here
        return nullptr;
    }

    std::vector<double> correlation_coefficients(std::vector<Vector> datasets, int nr_of_threads)
    {
        std::vector<double> result{};
        const size_t n = datasets.size();

        // reserve space for the result vector to avoid multiple allocations
        result.reserve(n * (n - 1) / 2);

        // precompute means
        std::vector<double> means(n);

        // [A] create threads
        pthread_t threads[nr_of_threads];
        int thread_ids[nr_of_threads];

        int datapoints_per_thread = n / nr_of_threads;
        int remaining_datapoints = n % nr_of_threads;

        ThreadData thread_data[nr_of_threads];

        for (int i = 0; i < nr_of_threads; i++)
        {
            thread_ids[i] = i;
            thread_data[i].thread_id = i;
            thread_data[i].n = n;
            thread_data[i].datasets = &datasets;
            thread_data[i].means = &means;
            thread_data[i].result = &result;

            thread_data[i].startpoint = i * datapoints_per_thread;
            thread_data[i].endpoint = (i + 1) * datapoints_per_thread;

            if (i == nr_of_threads - 1)
            {
                thread_data[i].endpoint += remaining_datapoints; // last thread takes the remainder
            }

            pthread_create(&threads[i], nullptr, thread_function, (void *)&thread_data[i]);
        }

        // [A] wait for threads to finish
        for (int i = 0; i < nr_of_threads; i++)
        {
            pthread_join(threads[i], nullptr);
        }

        return result;
    }

}