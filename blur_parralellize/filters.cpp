/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "filters.hpp"
#include "matrix.hpp"
#include "ppm.hpp"
#include <cmath>

#include <immintrin.h>
#include <sleef.h>
#include <pthread.h>

namespace Filter
{

    namespace Gauss
    { // [A] n = radius (15)
        void get_weights(int n, double *weights_out)
        {
            for (auto i{0}; i <= n; i++)
            {
                double x{static_cast<double>(i) * max_x / n};
                weights_out[i] = exp(-x * x * pi);
            }
        }
    }

    // Declare a global barrier
    pthread_barrier_t barrier;

    // [I] Thread function
    void *thread_function(void *arg)
    {
        ThreadData *data = static_cast<ThreadData *>(arg);
        Matrix *scratch = data->scratch;
        Matrix *dst = data->dst;
        double *w = data->weights;
        int radius = data->radius;
        int start_row = data->start_row;
        int end_row = data->end_row;


        const unsigned char* dst_R = dst->get_R();
        const unsigned char* dst_G = dst->get_G();
        const unsigned char* dst_B = dst->get_B();
        unsigned char* scratch_R = const_cast<unsigned char*>(scratch->get_R());
        unsigned char* scratch_G = const_cast<unsigned char*>(scratch->get_G());
        unsigned char* scratch_B = const_cast<unsigned char*>(scratch->get_B());

        int x_size = dst->get_x_size();
        int y_size = dst->get_y_size();

        // Horizontal pass: process only assigned rows
        for (auto y{start_row}; y < end_row; y++)
        {
            for (auto x{0}; x < x_size; x++)
            {
                auto r{w[0] * dst_R[y * x_size + x]}, g{w[0] * dst_G[y * x_size + x]}, b{w[0] * dst_B[y * x_size + x]}, n{w[0]};

                for (auto wi{1}; wi <= radius; wi++)
                {
                    auto wc{w[wi]};
                    auto x2{x - wi};
                    if (x2 >= 0)
                    {
                        r += wc * dst_R[y * x_size + x2];
                        g += wc * dst_G[y * x_size + x2];
                        b += wc * dst_B[y * x_size + x2];
                        n += wc;
                    }
                    x2 = x + wi;
                    if (x2 < x_size)
                    {
                        r += wc * dst_R[y * x_size + x2];
                        g += wc * dst_G[y * x_size + x2];
                        b += wc * dst_B[y * x_size + x2];
                        n += wc;
                    }
                }
                scratch_R[y * x_size + x] = r / n;
                scratch_G[y * x_size + x] = g / n;
                scratch_B[y * x_size + x] = b / n;
            }
        }

        // Wait for all threads to finish the horizontal pass
        pthread_barrier_wait(&barrier);

        // Vertical pass: process only assigned rows
        for (auto y{start_row}; y < end_row; y++)
        {
            for (auto x{0}; x < x_size; x++)
            {
                auto r{w[0] * scratch_R[y * x_size + x]}, g{w[0] * scratch_G[y * x_size + x]}, b{w[0] * scratch_B[y * x_size + x]}, n{w[0]};
                for (auto wi{1}; wi <= radius; wi++)
                {
                    auto wc{w[wi]};
                    auto y2{y - wi};
                    if (y2 >= 0)
                    {
                        r += wc * scratch_R[y2 * x_size + x];
                        g += wc * scratch_G[y2 * x_size + x];
                        b += wc * scratch_B[y2 * x_size + x];
                        n += wc;
                    }
                    y2 = y + wi;
                    if (y2 < y_size)
                    {
                        r += wc * scratch_R[y2 * x_size + x];
                        g += wc * scratch_G[y2 * x_size + x];
                        b += wc * scratch_B[y2 * x_size + x];
                        n += wc;
                    }
                }
                const_cast<unsigned char*>(dst_R)[y * x_size + x] = r / n;
                const_cast<unsigned char*>(dst_G)[y * x_size + x] = g / n;
                const_cast<unsigned char*>(dst_B)[y * x_size + x] = b / n;
            }
        }

        return nullptr;
    }

    // [I] Main blur function
    Matrix blur(Matrix m, const int radius, const int nr_of_threads)
    {
        double w[radius + 1]{}; // we create an array to hold the weights
        Gauss::get_weights(radius, w);

        auto x_size = m.get_x_size();
        auto y_size = m.get_y_size();
        unsigned scratch_size;
        if (y_size > x_size)
        {
            scratch_size = y_size;
        }
        else
        {
            scratch_size = x_size;
        }
        Matrix scratch{scratch_size};

        auto dst{m};

        // [I] Pthreads variables
        pthread_t threads[nr_of_threads];
        int thread_ids[nr_of_threads];

        int rows_per_thread = m.get_y_size() / nr_of_threads;
        int remaining_rows = m.get_y_size() % nr_of_threads;

        // [I] Create a struct to hold the data for each thread
        struct ThreadData thread_data[nr_of_threads];

        // [I] Initialize the barrier
        pthread_barrier_init(&barrier, nullptr, nr_of_threads);

        // [I] Initialize thread IDs
        for (int i = 0; i < nr_of_threads; i++)
        {
            thread_ids[i] = i;
        }

        // [I] Create the threads
        for (int i = 0; i < nr_of_threads; i++)
        {
            thread_data[i].scratch = &scratch;
            thread_data[i].dst = &dst;
            thread_data[i].weights = w;
            thread_data[i].radius = radius;
            thread_data[i].start_row = i * rows_per_thread;
            thread_data[i].end_row = (i + 1) * rows_per_thread;

            if (i == nr_of_threads - 1)
            {
                // [I] Last thread takes the remaining rows
                thread_data[i].end_row += remaining_rows;
            }

            pthread_create(&threads[i], nullptr, thread_function, &thread_data[i]);
        }

        // [I] Wait for the threads to finish
        for (int i = 0; i < nr_of_threads; i++)
        {
            pthread_join(threads[i], nullptr);
        }

        // [I] Destroy the barrier
        pthread_barrier_destroy(&barrier);

        return dst;
    }

}
