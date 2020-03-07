//
// Created by alexander on 07.03.2020.
//

#ifndef MARS_CI_RANDOM_H
#define MARS_CI_RANDOM_H

#include <cstdlib>

namespace Random {
    void init(uint seed) {
        srand(seed);
    }

    double uniform() {
        return 2 * (double) std::rand() / (double) RAND_MAX - 1;
        // TODO(aryavorskiy): Implement another random generator
    }
}

#endif //MARS_CI_RANDOM_H
