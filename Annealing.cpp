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

void Annealing::setUpResultWriting(string fileName) {
    writeResultsToFile = true;
    resultWriter = ofstream(fileName);
}

void writeLine(string line) {
    resultWriter << line << "\n";
}

void writeBlock(float *mat, float *block, int blockSize) {
    for (int setIndex = 0; setIndex < blockSize; ++setIndex) {
        resultWriter << "Set #" << setIndex << "; Hamiltonian: "
                     << Annealing::hamiltonian(mat, &(block[Annealing::size * setIndex]))
                     << "; Data:\n";
        for (int j = 0; j < Annealing::size; ++j) {
            resultWriter << block[setIndex * Annealing::size + j] << " ";
        }
        resultWriter << endl;
    }
    resultWriter << endl;
}

void Annealing::onResultsWritten(string postfix) {
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
            float v = 2 * (float) rand() / (float) RAND_MAX - 1;
            matPtr[i * size + j] = v;
            matPtr[j * size + i] = v;
        }
        matPtr[i * size + i] = 0;
    }
}

float Annealing::hamiltonian(const float *mat, const float *set) {
    float ham = 0;
    for (int i = 0; i < size; i++)
        for (int j = i + 1; j < size; j++)
            ham += set[i] * set[j] * mat[i * size + j];
    return ham;
}


float Annealing::meanField(const float *mat, const float *set, int spinIndex) { // Returns /Phi_ind
    float mf = 0;
    for (int i = 0; i < size; ++i)
        mf += mat[spinIndex * size + i] * set[i];
    return mf;
}

float Annealing::probXi(const float *setX, const float *setY, int spinIndex) { // Returns P_ind
    return (1 + setX[spinIndex] * setY[spinIndex]) / 2.f;
}

float Annealing::prob(float *setX, float *setY, int *expExternal) { // Returns P
    float pr = 1;
    *expExternal = 0;
    for (int i = 0; i < size; ++i) {
        pr *= probXi(setX, setY, i);
        adjustPrecision(&pr, expExternal);
    }
    return pr == 0 ? 0 : pr * exp10f((*expExternal + interactionQuotient));
}

float Annealing::probDXi(float *setX, float *setY, int spinIndex, int *expExternal) { // Returns dP / dx_ind
    if (1 + setX[spinIndex] * setY[spinIndex] == 0)
        return 0;
    return prob(setX, setY, expExternal) * setY[spinIndex] / (1 + setX[spinIndex] * setY[spinIndex]);
}

bool
Annealing::iterate(float *mat, float *block, int setIndex, const vector<int> &link, int spinIndex, float t,
                   int *expExternal) {
    float sf = 0;
    sf += meanField(mat, &(block[setIndex * size]), spinIndex);
    for (int interaction : link)
        sf -= probDXi(&(block[setIndex * size]), &(block[interaction * size]), spinIndex, expExternal);

    float old = block[setIndex * size + spinIndex];
    block[setIndex * size + spinIndex] = t > 0 ? tanhf(-sf / t) :
                                         sf > 0 ? -1 : 1;
    return (block[setIndex * size + spinIndex] - old > 0)
           ? (block[setIndex * size + spinIndex] - old > threshold)
           : (old - block[setIndex * size + spinIndex] > threshold);
}

void Annealing::anneal(float *mat, float *block, int blockSize, float temp, float step,
                       bool *thrInactive, int *expExternal, vector<vector<int>> allLinks) {
    if (writeResultsToFile) {
        resultWriteMutex.lock();
        ostringstream sHeader = ostringstream();
        sHeader << "Started processing block from temperature " << temp << ":";
        writeLine(sHeader.str());
        writeBlock(mat, block, blockSize);
        resultWriteMutex.unlock();
    }
    int counter = 0;
    float t = temp;
    do {
        t -= step;
        bool cont = true;
        while (cont) {
            cont = false;
            for (int spin_index = 0; spin_index < size; ++spin_index) {
                for (int run_index = 0; run_index < blockSize; ++run_index) {
                    if (iterate(mat, block, run_index, allLinks[run_index],
                                spin_index, t, expExternal))
                        cont = true;
                }
            }
            counter++;
        }
    } while (t > 0);
    printMutex.lock();
    cout << temp;
    for (int run_index = 0; run_index < blockSize; ++run_index)
        cout << " " << hamiltonian(mat, &(block[run_index * size]));
    cout << " [" << counter << " iterations]"
         << endl;
    printMutex.unlock();
    if (writeResultsToFile) {
        resultWriteMutex.lock();
        ostringstream fHeader = ostringstream();
        fHeader << "Finished processing block; Start temperature was " << temp << "; Took " << counter
                << " iterations; block data:";
        writeLine(fHeader.str());
        writeBlock(mat, block, blockSize);
        resultWriteMutex.unlock();
    }
    *thrInactive = true;
}