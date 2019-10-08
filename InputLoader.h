//
// Created by alexander on 10/3/19.
//
#include <vector>

#ifndef MARS_2_INPUTLOADER_H
#define MARS_2_INPUTLOADER_H

using namespace std;

class InputLoader {
public:
    static float *loadMatFromTable(string filename, int *size); // Returns pointer to new mat

    static float *loadMatFromList(string filename, int *size); // Returns pointer to new mat

    static int loadBlock(string filename, float *blockAddr, int matSize); // Returns block size

    static vector<vector<int>> loadLinks(string filename);
};


#endif //MARS_2_INPUTLOADER_H
