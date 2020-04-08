//
// Created by alexander on 07.03.2020.
//

#ifndef MARS_CI_RANDOM_H
#define MARS_CI_RANDOM_H

#include <cstdlib>

/**
 * This namespace contains all operations required to work with random numbers.
 */
namespace Random {
    /**
     * Initialize random generator with specified seed.
     * @param seed Seed value
     */
    void init(uint seed) {
        srand(seed);
    }

    /**
     * Get uniformly distributed random value in specified bounds.
     * @param min Lower bound
     * @param max Upper bound
     * @return Random value
     */
    double uniform(double min, double max) {
        return (max - min) * (double) std::rand() / (double) RAND_MAX + min;
        // TODO(aryavorskiy): Implement another random generator
    }
}

#endif //MARS_CI_RANDOM_H
