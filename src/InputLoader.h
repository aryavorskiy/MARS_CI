//
// Created by alexander on 10/3/19.
//
#include <vector>
#include <fstream>
#include <sstream>

#ifndef MARS_2_INPUTLOADER_H
#define MARS_2_INPUTLOADER_H

using namespace std;

class InputLoader {
public:
    static float *loadMatFromTable(const string &filename, int *size) {
        auto ifs = ifstream(filename);
        ifs >> *size;
        auto J = new float[*size * *size];
        for (int i = 0; i < *size; ++i)
            for (int j = 0; j < *size; ++j) {
                float tmp;
                ifs >> tmp;
                if (i <= j) {
                    J[i * *size + j] = tmp;
                    J[j * *size + i] = tmp;
                }
            }
        return J;
    }

    static float *loadMatFromList(const string &filename, int *size) {
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

    static int loadBlock(const string &filename, float *blockAddr, int matSize) {
        // blockSize will be written; matSize - mat size
        auto ifs = ifstream(filename);
        int blockSize;
        ifs >> blockSize;
        string line;
        getline(ifs, line);

        for (int confIndex = 0; confIndex < blockSize; ++confIndex) { // Read the block
            getline(ifs, line);
            if (line.empty()) // Ignore empty line
                confIndex -= 1;
            else if (line == "RAND") { // Randomize line
                for (int i = 0; i < matSize; ++i)
                    blockAddr[confIndex * matSize + i] = 2 * (float) rand() / (float) RAND_MAX - 1;

            } else { // Read line from line buffer
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

    static vector<vector<int>> loadLinks(const string &filename) {
        vector<vector<int>> outAllLinks = vector<vector<int>>();
        auto ifs = ifstream(filename);
        int blockSize;
        ifs >> blockSize;
        string line;
        getline(ifs, line);
        for (int lineIndex = 0; lineIndex < blockSize; ++lineIndex) {
            vector<int> links = vector<int>();
            getline(ifs, line);
            if (line == "EMPTY" || line == "NONE") { // Set interacts with no others
                outAllLinks.push_back(links);
                continue;
            } else if (line == "ALL") { // Set interacts with every other except himself
                for (int i = 0; i < blockSize; ++i)
                    if (i != lineIndex)
                        links.push_back(i);
                outAllLinks.push_back(links);
                continue;
            } else if (line == "NO_ANNEAL") {  // Set values will be unchanged during block annealing
                links.push_back(-1);
                outAllLinks.push_back(links);
                continue;
            }

            stringstream lineParser = stringstream(line); // Line needs parsing
            for (int j = 0; j < blockSize; ++j) {
                int buf;
                try {
                    lineParser >> buf;
                    if (links.empty() || buf != links.back()) // Duplicates are ignored
                        links.push_back(buf);
                } catch (exception &e) { // Invalid line --> ignore
                    j = blockSize;
                }
            }
            outAllLinks.push_back(links);
        }
        return outAllLinks;
    }
};

#endif //MARS_2_INPUTLOADER_H