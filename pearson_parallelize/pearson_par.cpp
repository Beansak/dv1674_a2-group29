/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// THIS IS FILE FOR PARRALLELIZED VERSION

#include "analysis_par.hpp"
#include "dataset.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char const* argv[])
{
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " [dataset] [outfile] [nr_of_threads]" << std::endl;
        std::exit(1);
    }

    auto datasets { Dataset::read(argv[1]) };
    auto nr_of_threads{std::stoi(argv[4])}; // Parse the number of threads
    auto corrs { Analysis::correlation_coefficients(datasets, nr_of_threads) };
    Dataset::write(corrs, argv[2]);

    return 0;
}
