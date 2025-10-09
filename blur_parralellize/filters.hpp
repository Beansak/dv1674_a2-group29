/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "matrix.hpp"

#if !defined(FILTERS_HPP)
#define FILTERS_HPP

namespace Filter
{
    // [I] Create a struct to hold the data for each thread
    struct ThreadData
    {
        Matrix *scratch;
        Matrix *dst;
        double *weights;
        int radius;
        int start_row;
        int end_row;
    };

    namespace Gauss
    {
        constexpr unsigned max_radius{1000};
        constexpr float max_x{1.33};
        constexpr float pi{3.14159};

        void get_weights(int n, double *weights_out);
        
    }
    void *thread_function(void *arg);
    Matrix blur(Matrix m, const int radius, const int nr_of_threads);

}

#endif