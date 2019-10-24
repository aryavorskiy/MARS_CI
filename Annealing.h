//
// Created by alexander on 09.10.2019.
//

#ifndef MARS_2_ANNEALING_H
#define MARS_2_ANNEALING_H

using namespace std;

namespace Annealing {
    extern int size;
    extern float interactionQuotient;

    float meanField(const float *mat, const float *set, int spinIndex);

    float prob(const float *setX, const float *setY, int *expExternal);

    float probDXi(float probGiven, const float *setX, const float *setY, int spinIndex);

    bool
    iterateSet(float *mat, float *block, int setIndex, const vector<int> &links, float currentTemp,
               int *expExternal);

    void setUpResultWriting(const string &fileName);

    void onResultsWritten(const string &postfix = "");

    void setRandomize(float *setPtr);

    void matRandomize(float *matPtr);

    float hamiltonian(const float *mat, const float *set);

    void anneal(float *mat, float *block, int blockSize, float startTemp, float tempStep, bool *thrInactive,
                int *expExternal,
                vector<vector<int>> allLinks);
}

#endif //MARS_2_ANNEALING_H