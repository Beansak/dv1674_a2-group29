/*
Author: David Holmqvist daae19@student.bth.se
*/

#include "analysis.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>

namespace Analysis
{

    std::vector<double> correlation_coefficients(std::vector<Vector> datasets)
    {
        std::vector<double> result{};
        const size_t n = datasets.size();

        // reserve space for the result vector to avoid multiple allocations
        result.reserve(n * (n - 1) / 2);

        // precompute means
        std::vector<double> means(n);
        for (size_t i = 0; i < n; i++)
        {
            means[i] = datasets[i].mean();
        }

        // precompute magnitudes
        std::vector<double> magnitudes(n);
        std::vector<Vector> mm_v(n);

        for (size_t i = 0; i < n; i++)
        {
            magnitudes[i] = (datasets[i] - means[i]).magnitude();
            mm_v[i] = (datasets[i] - means[i]);
        }

        for (size_t sample1 = 0; sample1 < n - 1; sample1++)
        {
            for (size_t sample2 = sample1 + 1; sample2 < n; sample2++)
            {
                auto x_mm_over_x_mag = mm_v[sample1] / magnitudes[sample1];
                auto y_mm_over_y_mag = mm_v[sample2] / magnitudes[sample2];

                auto r = x_mm_over_x_mag.dot(y_mm_over_y_mag);
                result.push_back(std::max(std::min(r, 1.0), -1.0));
            }
        }
        return result;
    }

    // Note: This function is not used in the current implementation
    double pearson(Vector vec1, Vector vec2)
    {
        auto x_mean{vec1.mean()};
        auto y_mean{vec2.mean()};

        auto x_mm{vec1 - x_mean};
        auto y_mm{vec2 - y_mean};

        auto x_mag{x_mm.magnitude()};
        auto y_mag{y_mm.magnitude()};

        auto x_mm_over_x_mag{x_mm / x_mag};
        auto y_mm_over_y_mag{y_mm / y_mag};

        auto r{x_mm_over_x_mag.dot(y_mm_over_y_mag)};

        return std::max(std::min(r, 1.0), -1.0);
    }
};