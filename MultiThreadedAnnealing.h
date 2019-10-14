//
// Created by alexander on 14.10.2019.
//

#ifndef MARS_2_MULTITHREADEDANNEALING_H
#define MARS_2_MULTITHREADEDANNEALING_H


#include <vector>
#include <sstream>
#include <iostream>
#include <thread>
#include "Annealing.h"

using namespace std;

bool *runningFlags;

namespace Annealing {
    void iterateUntil(float *mat, float *block, int setIndex, const vector<int> &link, int spinIndex, float t,
                      int *expExternal, bool *runningFlag) {
        while (Annealing::iterate(mat, block, setIndex, link, spinIndex, t, expExternal)) { // Do nothing
        }
        *runningFlag = false;
    }

    void annealAsync(float *mat, float *block, int blockSize, float temp, float step,
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
            for (int spinIndex = 0; spinIndex < size; ++spinIndex) {
                for (int setIndex = 0; setIndex < blockSize; ++setIndex) {
                    runningFlags[setIndex] = true;
                    thread(iterateUntil, mat, block, setIndex, allLinks[setIndex],
                           spinIndex, t, expExternal, runningFlags + setIndex).detach();
                }
                bool flag = true;
                while (flag) { // Wait while all async annealing operations finish work
                    flag = false;
                    for (int runIndex = 0; runIndex < blockSize; ++runIndex)
                        flag = flag || runningFlags[runIndex];
                }
            }
        } while (t > 0);
        printMutex.lock();
        cout << temp;
        for (int setIndex = 0; setIndex < blockSize; ++setIndex)
            cout << " " << hamiltonian(mat, &(block[setIndex * size]));
        cout << " [" << counter << " iterations]"
             << endl;
        printMutex.unlock();
        if (writeResultsToFile) {
            resultWriteMutex.lock();
            ostringstream fHeader = ostringstream();
            fHeader << "Finished processing block with async annealing; Start temperature was " << temp
                    << "; block data:";
            writeLine(fHeader.str());
            writeBlock(mat, block, blockSize);
            resultWriteMutex.unlock();
        }
        *thrInactive = true;
    }
}


#endif //MARS_2_MULTITHREADEDANNEALING_H
