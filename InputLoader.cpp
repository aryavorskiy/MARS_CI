//
// Created by alexander on 10/3/19.
//
#include <fstream>
#include <sstream>
#include "InputLoader.h"

float r = 2;

float *InputLoader::loadMatFromTable(string filename, int *size) {
    auto ifs = ifstream(filename);
    ifs >> *size;
    auto J = new float[*size * *size];
    for (int i = 0; i < *size; ++i) {
        for (int j = 0; j < *size; ++j) {
            float tmp;
            ifs >> tmp;
            if (i <= j) {
                J[i * *size + j] = tmp;
                J[j * *size + i] = tmp;
            }
        }
    }
    return J;
}

float *InputLoader::loadMatFromList(string filename, int *size) {
    ifstream ifs(filename);
    int edgeCount;
    ifs >> *size >> edgeCount;
    auto J = new float[*size * *size];
    for (int k = 0; k < *size * *size; ++k)
        J[k] = 0;

    for (int k = 0; k < edgeCount; ++k) {
        int i;
        int j;
        float tmp;
        ifs >> i >> j >> tmp;
        J[i * *size + j] = tmp;
        J[j * *size + i] = tmp;
    }
    return J;
}

int InputLoader::loadBlock(string filename, float *blockAddr, int matSize) {
    // blockSize will be written; matSize - mat size
    auto ifs = ifstream(filename);
    int blockSize, blockCount;
    ifs >> blockSize >> blockCount;

    // Read the block
    for (int confIndex = 0; confIndex < blockSize; ++confIndex) {
        string line;
        getline(ifs, line);
        if (line == "")
            confIndex -= 1; // Ignore empty line
        else if (line == "RAND") {
            // Randomize line
            for (int i = 0; i < matSize; ++i)
                blockAddr[confIndex * matSize + i] = 2 * (float) rand() / (float) RAND_MAX - 1;

        } else {
            // Read line from linebuffer
            stringstream lineParser(line);
            for (int i = 0; i < matSize; ++i) {
                float buf;
                lineParser >> buf;
                blockAddr[confIndex * matSize + i] = buf;
            }
        }
    }
    return blockSize;
}

vector<vector<int>> InputLoader::loadLinks(string filename) {
    vector<vector<int>> outAllLinks = vector<vector<int>>();
    auto ifs = ifstream(filename);
    int blockSize;
    ifs >> blockSize;
    string line;
    getline(ifs, line);
    for (int lineIndex = 0; lineIndex < blockSize; ++lineIndex) {
        vector<int> links = vector<int>();
        getline(ifs, line);
        stringstream lineParser = stringstream(line);
        for (int j = 0; j < blockSize; ++j) {
            int buf;
            try {
                lineParser >> buf;
                if (!links.empty() && buf == links.back())
                    j = blockSize;
                else
                    links.push_back(buf);
            }
            catch (exception) {
                j = blockSize;
            }
        }
        outAllLinks.push_back(links);
    }
    return outAllLinks;
}
