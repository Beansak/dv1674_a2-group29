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

        // precompute normalized vectors (mean-subtracted and divided by magnitude)
        std::vector<Vector> normalized;
        normalized.reserve(n);

        for (size_t i = 0; i < n; i++)
        {
            auto mm = datasets[i] - means[i];
            auto mag = mm.magnitude();
            normalized.push_back(mm / mag); // Store the fully normalized vector
        }

        for (size_t sample1 = 0; sample1 < n - 1; sample1++)
        {
            for (size_t sample2 = sample1 + 1; sample2 < n; sample2++)
            {
                auto r = normalized[sample1].dot(normalized[sample2]);
                result.push_back(std::max(std::min(r, 1.0), -1.0));
            }
        }
        return result;
    }
}