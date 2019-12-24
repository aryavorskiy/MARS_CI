//
// Created by alexander on 09.10.2019.
//

#ifndef MARS_2_ANNEALING_H
#define MARS_2_ANNEALING_H

#include "BigFloat.h"

using namespace std;

namespace Annealing {
    extern int size;
    extern BigFloat interactionQuotient;

    float meanField(const float *mat, const float *set, int spinIndex);

    BigFloat prob(const float *setX, const float *setY);

    bool iterateSet(float *mat, float *block, int setIndex, const vector<int> &links, float currentTemp,
                    bool hamiltonianLogMode);

    void setRandomize(float *setPtr);

    void matRandomize(float *matPtr);

    float hamiltonian(const float *mat, const float *set);

    void anneal(float *mat, float *block, int blockSize, float startTemp, float tempStep, bool *thrInactive,
                vector<vector<int>> allLinks, bool hamiltonianMode);

    float
    interactionField(const float *block, int spinIndex, int setIndex, int linkIndex, bool hamiltonianLogMode,
                     BigFloat prob);
}

#endif //MARS_2_ANNEALING_H