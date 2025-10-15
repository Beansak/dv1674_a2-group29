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
            for (auto i{0}; i <= n; i++)
            {
                double x{static_cast<double>(i) * max_x / n};
                weights_out[i] = exp(-x * x * pi);
            }
        }
    }

    Matrix blur(Matrix m, const int radius)
    {
        Matrix scratch{PPM::max_dimension};
        auto dst{m};

        // [I] Direct pointers to data arrays
        const unsigned char* dst_R = dst.get_R();
        const unsigned char* dst_G = dst.get_G();
        const unsigned char* dst_B = dst.get_B();
        unsigned char* scratch_R = const_cast<unsigned char*>(scratch.get_R());
        unsigned char* scratch_G = const_cast<unsigned char*>(scratch.get_G());
        unsigned char* scratch_B = const_cast<unsigned char*>(scratch.get_B());

        const auto x_size = dst.get_x_size();
        const auto y_size = dst.get_y_size();

        for (auto x{0}; x < x_size; x++)
        {
            for (auto y{0}; y < y_size; y++)
            {
                double w[Gauss::max_radius]{}; // we create an array to hold the weights
                Gauss::get_weights(radius, w);

                // unsigned char Matrix::r(unsigned x, unsigned y) const
                // {
                //     return R[y * x_size + x];
                // }

                // [A] define rgb and n (normalization factor) with the center pixel
                auto r{w[0] * dst_R[y * x_size + x]}, g{w[0] * dst_G[y * x_size + x]}, b{w[0] * dst_B[y * x_size + x]}, n{w[0]};

                
                // [A] loop through the weights and add the weighted values of the surrounding pixels
                for (auto wi{1}; wi <= radius; wi++)
                {
                    auto wc{w[wi]};
                    auto x2{x - wi};

                    // [A] check bounds and add the weighted pixel values to rgb and n
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

        for (auto x{0}; x < x_size; x++)
        {
            for (auto y{0}; y < y_size; y++)
            {
                double w[Gauss::max_radius]{};
                Gauss::get_weights(radius, w);

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

        return dst;
    }

}
