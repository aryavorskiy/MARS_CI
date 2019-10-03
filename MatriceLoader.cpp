//
// Created by alexander on 10/3/19.
//
#include <fstream>
#include "MatriceLoader.h"

float *MatriceLoader::loadAsMatrice(string filename, int *size) {
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

float *MatriceLoader::loadAsLattice(string filename, int *size) {
    auto ifs = ifstream(filename);
    ifs >> *size;
    auto J = new float[*size * *size];
    for (int k = 0; k < *size * *size; ++k)
        J[k] = 0;
    int edgeCount;
    ifs >> edgeCount;
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