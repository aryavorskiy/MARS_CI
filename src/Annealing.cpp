//
// Created by alexander on 09.10.2019.
//

#include <cmath>
#include "Annealing.h"
#include "OutputWriter.h"

using namespace std;

const float threshold = 0.001;

int Annealing::size = 100;

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
    for (int i = 0; i < size; i++)
        hamiltonian += set[i] * mat[i * size + i];//*/
    return hamiltonian;
}

float Annealing::meanField(const float *mat, const float *set, int spinIndex) {  // Returns /Phi_ind
    float meanField = 0;
    for (int i = 0; i < size; ++i)
        if (i != spinIndex)
            meanField += mat[spinIndex * size + i] * set[i];
        else
            meanField += mat[spinIndex * size + i];
    return meanField;
}

bool Annealing::iterateSet(float *mat, float *block, int setIndex, float currentTemp) {
    bool setNotStable = false;

    for (int spinIndex = 0; spinIndex < size; ++spinIndex) {
        // Calculate field
        float lMeanField = meanField(mat, block + setIndex * size, spinIndex);

        // Write new value to spin
        float old = block[setIndex * size + spinIndex];
        block[setIndex * size + spinIndex] = currentTemp > 0 ? tanhf(-lMeanField / currentTemp) :
                                             lMeanField > 0 ? -1 : 1;
        setNotStable = setNotStable || ((block[setIndex * size + spinIndex] - old > 0)
                                        ? (block[setIndex * size + spinIndex] - old > threshold)
                                        : (old - block[setIndex * size + spinIndex] > threshold));
    }
    return setNotStable;
}

void Annealing::anneal(float *mat, float *block, int blockSize, float startTemp, float tempStep, bool *thrInactive) {
    OutputWriter::outputResultsOnStart(mat, block, blockSize, startTemp);

    int stepCounter = 0;
    float currentTemp = startTemp;
    do {
        currentTemp -= tempStep;
        bool continueAnnealing = true;
        while (continueAnnealing) {
            continueAnnealing = false;
            for (int setIndex = 0; setIndex < blockSize; ++setIndex)
                continueAnnealing = continueAnnealing || iterateSet(mat, block, setIndex, currentTemp);
            stepCounter++;
        }
        OutputWriter::outputResultsIntermediate(startTemp, currentTemp);
    } while (currentTemp > 0);

    OutputWriter::outputResultsOnFinish(mat, block, blockSize, startTemp, size, stepCounter);
    *thrInactive = true;
}
