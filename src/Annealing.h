//
// Created by alexander on 09.10.2019.
//

#ifndef MARS_2_ANNEALING_H
#define MARS_2_ANNEALING_H

#include "BigFloat.h"

namespace Annealing {
    extern int size;
    extern BigFloat interactionQuotient;
    extern float temperatureInteractionThreshold;

    float meanField(const float *mat, const float *set, int spinIndex);

    BigFloat prob(const float *setX, const float *setY);

    bool iterateSet(float *mat, float *block, int setIndex, const std::vector<int> &links, float currentTemp,
                    bool hamiltonianLogMode);

    void setRandomize(float *setPtr);

    void matRandomize(float *matPtr);

    float hamiltonian(const float *mat, const float *set);

    void anneal(float *mat, float *block, int blockSize, float startTemp, float tempStep, bool *thrInactive,
                std::vector<std::vector<int>> allLinks, bool hamiltonianMode);

    float
    interactionField(const float *block, int spinIndex, int setIndex, int linkIndex, bool hamiltonianLogMode,
                     BigFloat prob);
}

#endif //MARS_2_ANNEALING_H