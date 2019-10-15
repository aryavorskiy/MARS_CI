//
// Created by alexander on 09.10.2019.
//

#include <vector>
#include <mutex>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Annealing.h"

using namespace std;

mutex resultWriteMutex;
mutex printMutex;
const float threshold = 0.001;
ofstream resultWriter;
bool writeResultsToFile = false;

int Annealing::size = 100;
float Annealing::interactionQuotient = 0;

void adjustPrecision(float *mantissa, int *exponent) {
    if (*mantissa == 0) {
        *exponent = 0;
        return;
    }
    while (abs(*mantissa) > 10) {
        *mantissa /= 10;
        *exponent += 1;
    }
    while (abs(*mantissa) < 1) {
        *mantissa *= 10;
        *exponent -= 1;
    }
}

void Annealing::setUpResultWriting(const string &fileName) {
    writeResultsToFile = true;
    resultWriter = ofstream(fileName);
}

void writeLine(const string &line) {
    resultWriter << line << "\n";
}

void writeBlock(float *mat, float *block, int blockSize) {
    for (int setIndex = 0; setIndex < blockSize; ++setIndex) {
        resultWriter << "Set #" << setIndex << "; Hamiltonian: "
                     << Annealing::hamiltonian(mat, block + Annealing::size * setIndex)
                     << "; Data:\n";
        for (int j = 0; j < Annealing::size; ++j) {
            resultWriter << block[setIndex * Annealing::size + j] << " ";
        }
        resultWriter << endl;
    }
    resultWriter << endl;
}

void Annealing::onResultsWritten(const string &postfix) {
    if (writeResultsToFile) {
        resultWriter << postfix;
        resultWriter.close();
    }
}

void Annealing::setRandomize(float *setPtr) {
    for (int i = 0; i < size; ++i) {
        setPtr[i] = rand() / (float) RAND_MAX * 2 - 1;
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

float Annealing::hamiltonian(const float *mat, const float *set) { // Returns hamiltonian of given system
    float hamiltonian = 0;
    for (int i = 0; i < size; i++)
        for (int j = i + 1; j < size; j++)
            hamiltonian += set[i] * set[j] * mat[i * size + j];
    return hamiltonian;
}


float Annealing::meanField(const float *mat, const float *set, int spinIndex) { // Returns /Phi_ind
    float meanField = 0;
    for (int i = 0; i < size; ++i)
        meanField += mat[spinIndex * size + i] * set[i];
    return meanField;
}

float Annealing::probXi(const float *setX, const float *setY, int spinIndex) { // Returns P_ind
    return (1 + setX[spinIndex] * setY[spinIndex]) / 2.f;
}

float Annealing::prob(float *setX, float *setY, int *expExternal) { // Returns P
    float eqProbability = 1;
    *expExternal = 0;
    for (int i = 0; i < size; ++i) {
        eqProbability *= probXi(setX, setY, i);
        adjustPrecision(&eqProbability, expExternal);
    }
    return eqProbability == 0 ? 0 : eqProbability * exp10f(((float) *expExternal + interactionQuotient));
}

float Annealing::probDXi(float *setX, float *setY, int spinIndex, int *expExternal) { // Returns dP / dx_ind
    if (1 + setX[spinIndex] * setY[spinIndex] == 0)
        return 0;
    return prob(setX, setY, expExternal) * setY[spinIndex] / (1 + setX[spinIndex] * setY[spinIndex]);
}

bool
Annealing::iterate(float *mat, float *block, int setIndex, const vector<int> &link, int spinIndex, float currentTemp,
                   int *expExternal) {
    float totalField = 0;
    totalField += meanField(mat, block + setIndex * size, spinIndex);
    for (int interaction : link)
        totalField -= probDXi(block + setIndex * size, block + interaction * size, spinIndex, expExternal);

    float old = block[setIndex * size + spinIndex];
    block[setIndex * size + spinIndex] = currentTemp > 0 ? tanhf(-totalField / currentTemp) :
                                         totalField > 0 ? -1 : 1;
    return (block[setIndex * size + spinIndex] - old > 0)
           ? (block[setIndex * size + spinIndex] - old > threshold)
           : (old - block[setIndex * size + spinIndex] > threshold);
}

void Annealing::anneal(float *mat, float *block, int blockSize, float startTemp, float tempStep,
                       bool *thrInactive, int *expExternal, vector<vector<int>> allLinks) {
    if (writeResultsToFile) { // Block annealing started, write to full log
        resultWriteMutex.lock();
        ostringstream sHeader = ostringstream();
        sHeader << "Started processing block from temperature " << startTemp << ":";
        writeLine(sHeader.str());
        writeBlock(mat, block, blockSize);
        resultWriteMutex.unlock();
    }

    int stepCounter = 0;
    float currentTemp = startTemp;
    do {
        currentTemp -= tempStep;
        bool continueAnnealing = true;
        while (continueAnnealing) {
            continueAnnealing = false;
            for (int spinIndex = 0; spinIndex < size; ++spinIndex) {
                for (int setIndex = 0; setIndex < blockSize; ++setIndex) {
                    if (iterate(mat, block, setIndex, allLinks[setIndex],
                                spinIndex, currentTemp, expExternal))
                        continueAnnealing = true;
                }
            }
            stepCounter++;
        }
        if (writeResultsToFile) { // Step complete, write to full log
            resultWriteMutex.lock();
            ostringstream stepComplete = ostringstream();
            stepComplete << "Annealing step complete: Start temperature " << startTemp << ", now " << currentTemp
                         << endl;
            writeLine(stepComplete.str());
            resultWriteMutex.unlock();
        }
    } while (currentTemp > 0);
    printMutex.lock();
    cout << startTemp;
    bool noInteraction = true;
    for (vector<int> link : allLinks)
        noInteraction = noInteraction || link.empty();
    for (int setIndex = 0; setIndex < blockSize; ++setIndex)
        if (!(allLinks[setIndex].empty() || noInteraction))
            cout << " " << hamiltonian(mat, block + setIndex * size);
        else
            cout << " <" << hamiltonian(mat, block + setIndex * size) << ">";
    cout << " [" << stepCounter << " iterations]"
         << endl;
    printMutex.unlock();

    if (writeResultsToFile) { // Block annealing complete, write to full log
        resultWriteMutex.lock();
        ostringstream fHeader = ostringstream();
        fHeader << "Finished processing block; Start temperature was " << startTemp << "; Took " << stepCounter
                << " steps; block data:";
        writeLine(fHeader.str());
        writeBlock(mat, block, blockSize);
        resultWriteMutex.unlock();
    }
    *thrInactive = true;
}