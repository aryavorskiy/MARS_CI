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

    float probXi(const float *setX, const float *setY, int spinIndex);

    float prob(float *setX, float *setY, int *expExternal);

    float probDXi(float *setX, float *setY, int spinIndex, int *expExternal);

    bool
    iterate(float *mat, float *block, int setIndex, const vector<int> &link, int spinIndex, float t, int *expExternal);

    void setUpResultWriting(string fileName);

    void onResultsWritten(string postfix = "");

    void setRandomize(float *setPtr);

    void matRandomize(float *matPtr);

    float hamiltonian(const float *mat, const float *set);

    void anneal(float *mat, float *block, int blockSize, float temp, float step, bool *thrInactive, int *expExternal,
                vector<vector<int>> allLinks);
}

#endif //MARS_2_ANNEALING_H