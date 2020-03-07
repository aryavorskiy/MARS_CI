//
// Created by alexander on 03.03.2020.
//

#ifndef MARS_CI_LATTICE_H
#define MARS_CI_LATTICE_H

#include <string>
#include <fstream>
#include "Random.h"

/**
 * Represents a bi-dimensional square lattice that describes spin interaction.
 * @tparam T
 */
template<typename T>
class Lattice {
private:
    int mat_size;
    T *mat_values;
public:
    /**
     * Default Lattice constructor.
     */
    Lattice() : mat_size(0), mat_values(nullptr) {};

    /**
     * Empty or random Lattice constructor.
     * @param _size Lattice size
     * @param randomize False to set all lattice elements to zero, True to randomize them
     */
    explicit Lattice(int _size, bool randomize = false);

    /**
     * Lattice constructor that loads element values from specified filename.
     * @param _filename Filename where Lattice values are stored
     */
    explicit Lattice(const std::string &_filename);

    /**
     * Get element value by indices.
     * @param x Column index
     * @param y Row index
     * @return Element value
     */
    T &operator()(int x, int y);

    /**
     * Get Lattice size.
     * @return Lattice size
     */
    int size();
};

template<typename T>
Lattice<T>::Lattice(const std::string &_filename) {
    auto ifs = std::ifstream(_filename);
    int k = 0;
    ifs >> k;
    mat_size = k;
    mat_values = new T[mat_size * mat_size];
    for (int i = 0; i < mat_size; ++i) {
        for (int j = 0; j < mat_size; ++j) {
            float tmp = 0;
            ifs >> tmp;
            if (i <= j) {
                mat_values[i * mat_size + j] = tmp;
                mat_values[j * mat_size + i] = tmp;
            }
        }
    }
}

template<typename T>
Lattice<T>::Lattice(int _size, bool randomize) {
    mat_size = _size;
    mat_values = new T[mat_size * mat_size];
    for (int i = 0; i < mat_size; ++i) {
        for (int j = 0; j < mat_size; ++j) {
            if (randomize and i > j) {
                mat_values[i * mat_size + j] = mat_values[j * mat_size + i] = Random::uniform(-1, 1);
            } else if (not randomize or i == j) {
                mat_values[i * mat_size + j] = 0;
            }
        }
    }
}

template<typename T>
T &Lattice<T>::operator()(int x, int y) {
    // TODO(aryavorskiy): Probably another operator should be used here
    return mat_values[x * mat_size + y];
}

template<typename T>
int Lattice<T>::size() {
    return mat_size;
}

#endif //MARS_CI_LATTICE_H
