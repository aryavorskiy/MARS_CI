//
// Created by alexander on 09.10.2019.
//

#include <vector>
#include <cmath>
#include "Annealing.h"
#include "BigFloat.h"
#include "OutputWriter.h"

using namespace std;

const float threshold = 0.001;
const float interactionThreshold = 0.9;

int Annealing::size = 100;
BigFloat Annealing::interactionQuotient = BigFloat();

void Annealing::setRandomize(float *setPtr) {
    for (int i = 0; i < size; ++i) {
        setPtr[i] = (float) rand() / (float) RAND_MAX * 2 - 1;
        // TODO(aryavorskiy): Implement a normal random generator (Clang-tidy doesn't like this one)
    }
}

void Annealing::matRandomize(float *matPtr) {
    for (int i = 0; i < size; ++i) {
        for (int j = i + 1; j < size; ++j) {
            float val = 2 * (float) rand() / (float) RAND_MAX - 1;
            matPtr[i * size + j] = val;
            matPtr[j * size + i] = val;
        }
        matPtr[i * size + i] = 0;
    }
}

float Annealing::hamiltonian(const float *mat, const float *set) {  // Returns hamiltonian of given system
    float hamiltonian = 0;
    for (int i = 0; i < size; i++)
        for (int j = i + 1; j < size; j++)
            hamiltonian += set[i] * set[j] * mat[i * size + j];
    return hamiltonian;
}

float Annealing::meanField(const float *mat, const float *set, int spinIndex) {  // Returns /Phi_ind
    float meanField = 0;
    for (int i = 0; i < size; ++i)
        meanField += mat[spinIndex * size + i] * set[i];
    return meanField;
}

BigFloat Annealing::prob(const float *setX, const float *setY) {  // Returns P
    BigFloat eqProbability(1);
    for (int spinIndex = 0; spinIndex < size; ++spinIndex) {
        eqProbability *= (1 + setX[spinIndex] * setY[spinIndex]) / 2.f;
    }
    return eqProbability;
}

float
Annealing::interactionField(const float *block, int spinIndex, int setIndex, int linkIndex, bool hamiltonianLogMode,
                            BigFloat prob) {
    if (block[spinIndex + setIndex * size] * block[spinIndex + linkIndex * size] == -1)
        return 0;
    if (abs(block[spinIndex + setIndex * size]) > interactionThreshold)
        return 0.;
    if (hamiltonianLogMode) {
        if (abs(block[spinIndex + linkIndex * size]) >= 1)
            return (MAXFLOAT * block[spinIndex + linkIndex * size]);
        float under_log_fraction = 1 + block[spinIndex + linkIndex * size];
        under_log_fraction = under_log_fraction / (1 - block[spinIndex + linkIndex * size]);
        return (float) (interactionQuotient / 2 * logf(under_log_fraction));
    } else
        return (float) (interactionQuotient * (block[spinIndex + linkIndex * size] /
                                               (1 + block[spinIndex + setIndex * size] *
                                                    block[spinIndex + linkIndex * size])) * prob);
}

bool Annealing::iterateSet(float *mat, float *block, int setIndex, const vector<int> &links, float currentTemp,
                           bool hamiltonianLogMode) {
    auto *probStorage = new BigFloat[links.size()];
    for (ulong i = 0; i < links.size(); i++)
        probStorage[i] = prob(block + setIndex * size, block + links[i] * size);
    bool setNotStable = false;

    for (int spinIndex = 0; spinIndex < size; ++spinIndex) {
        // Calculate field
        BigFloat totalField(0);
        if (currentTemp > 0)
            for (ulong i = 0; i < links.size(); i++) { // Evaluate linked sets interactions
                float tField = interactionField(block, spinIndex, setIndex, links[i], hamiltonianLogMode,
                                                probStorage[i]);
                totalField += tField;
            }
        totalField += meanField(mat, block + setIndex * size, spinIndex);

        // Write new value to spin
        float old = block[setIndex * size + spinIndex];
        block[setIndex * size + spinIndex] = currentTemp > 0 ? tanhf((float) (totalField / -currentTemp)) :
                                             totalField > 0 ? -1 : 1;
        setNotStable = setNotStable || ((block[setIndex * size + spinIndex] - old > 0)
                                        ? (block[setIndex * size + spinIndex] - old > threshold)
                                        : (old - block[setIndex * size + spinIndex] > threshold));
        if (abs(old - block[setIndex * size + spinIndex]) >= threshold)
            int a = 0;  // DEBUG

        for (ulong i = 0; i < links.size(); i++)  // Recalculate P values
            if (1 + old * block[links[i] * size + spinIndex] != 0)
                probStorage[i] *= (1 + block[setIndex * size + spinIndex] * block[links[i] * size + spinIndex]) /
                                  (1 + old * block[links[i] * size + spinIndex]);
    }
    delete[] probStorage;  // Clean up to prevent memory leaks
    return setNotStable;
}

void Annealing::anneal(float *mat, float *block, int blockSize, float startTemp, float tempStep, bool *thrInactive,
                       vector<vector<int>> allLinks, bool hamiltonianMode) {
    OutputWriter::outputResultsOnStart(mat, block, blockSize, startTemp);

    int stepCounter = 0;
    float currentTemp = startTemp;
    do {
        currentTemp -= tempStep;
        bool continueAnnealing = true;
        while (continueAnnealing) {
            continueAnnealing = false;
            for (int setIndex = 0; setIndex < blockSize; ++setIndex)
                if (allLinks[setIndex].empty() || allLinks[setIndex][0] != -1)  // Otherwise NO_ANNEAL is true for set
                    continueAnnealing = iterateSet(mat, block, setIndex, allLinks[setIndex],
                                                   currentTemp, hamiltonianMode) || continueAnnealing;
            stepCounter++;
        }
        OutputWriter::outputResultsIntermediate(startTemp, currentTemp);
    } while (currentTemp > 0);

    OutputWriter::outputResultsOnFinish(mat, block, blockSize, startTemp, size, allLinks, stepCounter);
    *thrInactive = true;
}
