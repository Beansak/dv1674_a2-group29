/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "filters.hpp"
#include "matrix.hpp"
#include "ppm.hpp"
#include <cmath>

#include <immintrin.h>
#include <sleef.h>

namespace Filter
{

    namespace Gauss
    { // [A] n = radius (15)
        void get_weights(int n, double *weights_out)
        {
            //[I]SIMD AVX version with scalar tail
            const int step = 4;
            //[I]Creating vectors that holds the constants we need for the calculations
            __m256d v_max_x = _mm256_set1_pd(static_cast<double>(max_x));
            __m256d v_pi = _mm256_set1_pd(static_cast<double>(pi));
            __m256d v_n = _mm256_set1_pd(static_cast<double>(n));

            int i = 0;
            //[I]SIMD loop: process 4 elements at a time
            for (; i + step - 1 <= n; i += step)
            {
                //[I]Creating a vector that holds the current indices
                __m256d v_i = _mm256_set_pd(static_cast<double>(i + 3), static_cast<double>(i + 2), static_cast<double>(i + 1), static_cast<double>(i));
                __m256d v_x = _mm256_mul_pd(v_i, v_max_x); // x = i * max_x
                v_x = _mm256_div_pd(v_x, v_n);             // x = x / n

                __m256d v_x2 = _mm256_mul_pd(-v_x, v_x);     // -x * x
                __m256d v_x2_pi = _mm256_mul_pd(v_x2, v_pi); // -x * x * pi
                __m256d v_result = Sleef_expd4_u10(v_x2_pi); // exp(-x * x * pi)
                _mm256_storeu_pd(&weights_out[i], v_result); // store the result in the output array
            }

            //[I]Scalar tail: handle remaining elements
            for (; i <= n; i++)
            {
                double x{static_cast<double>(i) * max_x / n};
                weights_out[i] = exp(-x * x * pi);
            }
        }
    }

    Matrix blur(Matrix m, const int radius, const int nr_of_threads)
    {

        double w[Gauss::max_radius]{}; // we create an array to hold the weights
        Gauss::get_weights(radius, w);

        Matrix scratch{PPM::max_dimension};
        auto dst{m};

        // [I] Pthreads variables
        pthread_t threads[nr_of_threads];
        int thread_ids[nr_of_threads];


        int rows_per_thread = m.get_y_size() / nr_of_threads;
        int remaining_rows = m.get_y_size() % nr_of_threads;
        
        

        // [I] Create a struct to hold the data for each thread
        struct ThreadData thread_data[nr_of_threads];

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

        return dst;

    }


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


            

            for (auto y{start_row}; y < end_row; y++)
            {
                for (auto x{0}; x < dst->get_x_size(); x++)
                {

                    // unsigned char Matrix::r(unsigned x, unsigned y) const
                    // {
                    //     return R[y * x_size + x];
                    // }

                    // [A] define rgb and n (normalization factor) with the center pixel
                    auto r{w[0] * dst->r(x, y)}, g{w[0] * dst->g(x, y)}, b{w[0] * dst->b(x, y)}, n{w[0]};

                    // [A] loop through the weights and add the weighted values of the surrounding pixels
                    for (auto wi{1}; wi <= radius; wi++)
                    {
                        auto wc{w[wi]};
                        auto x2{x - wi};

                        // [A] check bounds and add the weighted pixel values to rgb and n
                        if (x2 >= 0)
                        {
                            r += wc * dst->r(x2, y);
                            g += wc * dst->g(x2, y);
                            b += wc * dst->b(x2, y);
                            n += wc;
                        }
                        x2 = x + wi;
                        if (x2 < dst->get_x_size())
                        {
                            r += wc * dst->r(x2, y);
                            g += wc * dst->g(x2, y);
                            b += wc * dst->b(x2, y);
                            n += wc;
                        }
                    }
                    scratch->r(x, y) = r / n;
                    scratch->g(x, y) = g / n;
                    scratch->b(x, y) = b / n;
                }
            }

            for (auto y{0}; y < dst->get_y_size(); y++)
            {
                for (auto x{0}; x < dst->get_x_size(); x++)
                {

                    auto r{w[0] * scratch->r(x, y)}, g{w[0] * scratch->g(x, y)}, b{w[0] * scratch->b(x, y)}, n{w[0]};

                    for (auto wi{1}; wi <= radius; wi++)
                    {
                        auto wc{w[wi]};
                        auto y2{y - wi};
                        if (y2 >= 0)
                        {
                            r += wc * scratch->r(x, y2);
                            g += wc * scratch->g(x, y2);
                            b += wc * scratch->b(x, y2);
                            n += wc;
                        }
                        y2 = y + wi;
                        if (y2 < dst->get_y_size())
                        {
                            r += wc * scratch->r(x, y2);
                            g += wc * scratch->g(x, y2);
                            b += wc * scratch->b(x, y2);
                            n += wc;
                        }
                    }
                    dst->r(x, y) = r / n;
                    dst->g(x, y) = g / n;
                    dst->b(x, y) = b / n;
                }
            }
            return nullptr;
        }

}
