//
// Created by alexander on 08.11.2019.
//

#ifndef MARS_CI_OUTPUTWRITER_H
#define MARS_CI_OUTPUTWRITER_H

#include <sstream>
#include <fstream>
#include <mutex>
#include <vector>
#include <iostream>
#include "Annealing.h"

using namespace std;


namespace OutputWriter {
    extern mutex resultWriteMutex;
    extern mutex printMutex;
    extern ofstream resultWriter;
    extern bool writeResultsToFile;

    void setUpResultWriting(const string &fileName);

    void writeLine(const string &line);

    void writeBlock(float *mat, float *block, int blockSize);

    void onResultsWritten(const string &postfix = "");

    void outputResultsOnStart(float *mat, float *block, int blockSize, float startTemp);

    void outputResultsIntermediate(float startTemp, float currentTemp);

    void outputResultsOnFinish(float *mat, float *block, int blockSize, float startTemp, int size, int stepCounter);
};


#endif //MARS_CI_OUTPUTWRITER_H
