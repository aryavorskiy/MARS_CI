//
// Created by alexander on 09.10.2019.
//

#include <vector>
#include "Annealing.h"
#include "BigFloat.h"
#include "OutputWriter.h"

using namespace std;

const float threshold = 0.001;

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

BigFloat
Annealing::probDXi(BigFloat probGiven, const float *setX, const float *setY, int spinIndex) {  // Returns dP / dx_ind
    if (1 + setX[spinIndex] * setY[spinIndex] == 0)
        return BigFloat();
    return probGiven * setY[spinIndex] / (1 + setX[spinIndex] * setY[spinIndex]);
}

bool Annealing::iterateSet(float *mat, float *block, int setIndex, const vector<int> &links, float currentTemp,
                           bool hamiltonianMode) {
    auto *probStorage = new BigFloat[links.size()];
    for (ulong i = 0; i < links.size(); i++)
        probStorage[i] = prob(block + setIndex * size, block + links[i] * size);
    bool setNotStable = false;

    for (int spinIndex = 0; spinIndex < size; ++spinIndex) {
        BigFloat totalField(meanField(mat, block + setIndex * size, spinIndex));
        for (ulong i = 0; i < links.size(); i++) {  // Evaluate linked sets
            if (block[spinIndex + setIndex * size] * block[spinIndex + links[i] * size] == -1)
                continue;  // Prevent zero division
            if (hamiltonianMode)  // Logarithm in hamiltonian
                totalField -= BigFloat(interactionQuotient) * (block[spinIndex + links[i] * size] /
                                                               (1 + block[spinIndex + setIndex * size] *
                                                                    block[spinIndex + links[i] * size]));
            else  // No logarithm in hamiltonian
                totalField -= probDXi(probStorage[i], block + setIndex * size, block + links[i] * size,
                                      spinIndex) * interactionQuotient;
        }

        float old = block[setIndex * size + spinIndex];
        block[setIndex * size + spinIndex] = currentTemp > 0 ? tanhf((float) (totalField / -currentTemp)) :
                                             totalField > 0 ? -1 : 1;
        setNotStable = setNotStable || ((block[setIndex * size + spinIndex] - old > 0)
                                        ? (block[setIndex * size + spinIndex] - old > threshold)
                                        : (old - block[setIndex * size + spinIndex] > threshold));

        for (ulong i = 0; i < links.size(); i++)
            if (1 + old * block[links[i] * size + spinIndex] != 0)
                probStorage[i] *= (1 + block[setIndex * size + spinIndex] * block[links[i] * size + spinIndex]) /
                                  (1 + old * block[links[i] * size + spinIndex]);
    }
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
                    continueAnnealing = continueAnnealing || iterateSet(mat, block, setIndex, allLinks[setIndex],
                                                                        currentTemp, hamiltonianMode);
            stepCounter++;
        }
        OutputWriter::outputResultsIntermediate(startTemp, currentTemp);
    } while (currentTemp > 0);

    OutputWriter::outputResultsOnFinish(mat, block, blockSize, startTemp, size, allLinks, stepCounter);
    *thrInactive = true;
}
