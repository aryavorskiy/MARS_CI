//
// Created by alexander on 09.10.2019.
//

#ifndef MARS_2_ANNEALING_H
#define MARS_2_ANNEALING_H

using namespace std;

namespace Annealing {
    extern int size;

    float meanField(const float *mat, const float *set, int spinIndex);

    void setRandomize(float *setPtr);

    void matRandomize(float *matPtr);

    float hamiltonian(const float *mat, const float *set);

    void anneal(float *mat, float *block, int blockSize, float startTemp, float tempStep, bool *thrInactive);

    bool iterateSet(float *mat, float *block, int setIndex, float currentTemp);
}

#endif //MARS_2_ANNEALING_H