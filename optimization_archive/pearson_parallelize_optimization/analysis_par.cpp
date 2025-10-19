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

        for (size_t i = data->precompute_start; i < data->precompute_end; i++)
        {
            (*data->means)[i] = (*data->datasets)[i].mean();

            auto mm = (*data->datasets)[i] - (*data->means)[i];
            auto mag = mm.magnitude();
            (*data->normalized)[i] = mm / mag; // Store the fully normalized vector
        }


        pthread_barrier_wait(data->barrier);

        for (int pair_index = data->correlation_start; pair_index < data->correlation_end; pair_index++)
        {
            int sample1 = 0;
            int temp_index = pair_index;

            while(temp_index >= (data->n - 1 -sample1))
            {
                temp_index -= (data->n - 1 - sample1);
                sample1++;
            }

            int sample2 = sample1 + 1 + temp_index;
            auto r = data->normalized->at(sample1).dot(data->normalized->at(sample2));
            auto clamped_r = std::max(std::min(1.0, r), -1.0);

            (*data->result)[pair_index] = clamped_r;
        }

        return nullptr;
    }

    std::vector<double> correlation_coefficients(std::vector<Vector> datasets, int nr_of_threads)
    {
        
        const size_t n = datasets.size();
        const size_t total_pairs = n * (n - 1) / 2;

        // [I] Prepare result storage and means and normalized vectors
        std::vector<double> result(total_pairs);
        std::vector<double> means(n);
        std::vector<Vector> normalized(n);

        // [I] Initialize the barrier
        pthread_barrier_t barrier;
        pthread_barrier_init(&barrier, nullptr, nr_of_threads);

        // [A] create threads
        pthread_t threads[nr_of_threads];
        ThreadData thread_data[nr_of_threads];

        int datasets_per_thread = n / nr_of_threads;
        int remaining_datasets = n % nr_of_threads;
        int pairs_per_thread = total_pairs / nr_of_threads;
        int remaining_pairs = total_pairs % nr_of_threads;

        
        for (int i = 0; i < nr_of_threads; i++)
        {
            thread_data[i].thread_id = i;
            thread_data[i].nr_of_threads = nr_of_threads;
            thread_data[i].n = n;
            thread_data[i].datasets = &datasets;
            thread_data[i].normalized = &normalized;
            thread_data[i].means = &means;
            thread_data[i].result = &result;
            thread_data[i].barrier = &barrier;

            thread_data[i].precompute_start = i * datasets_per_thread;
            thread_data[i].precompute_end = (i + 1) * datasets_per_thread;

            if (i == nr_of_threads - 1)
            {
                thread_data[i].precompute_end += remaining_datasets; // last thread takes the remainder
            }

            thread_data[i].correlation_start = i * pairs_per_thread;
            thread_data[i].correlation_end = (i + 1) * pairs_per_thread;

            if (i == nr_of_threads - 1)
            {
                thread_data[i].correlation_end += remaining_pairs; // last thread takes the remainder
            }

            pthread_create(&threads[i], nullptr, thread_function, (void *)&thread_data[i]);
        }

        // [A] wait for threads to finish
        for (int i = 0; i < nr_of_threads; i++)
        {
            pthread_join(threads[i], nullptr);
        }

        pthread_barrier_destroy(&barrier);

        return result;
    }

}