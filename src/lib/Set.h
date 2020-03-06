//
// Created by alexander on 03.03.2020.
//

#ifndef MARS_CI_SET_H
#define MARS_CI_SET_H

#include <iosfwd>
#include "Lattice.h"

enum SetType {
    INDEPENDENT,
    DEPENDENT,
    NO_ANNEAL,
    UNDEFINED,
    EMPTY
};

template<typename T>
class Set {
private:
    int set_size;
    T *set_values;
public:
    SetType set_type;

    Set() : set_type(EMPTY), set_size(0), set_values(nullptr) {}

    Set(int size) : set_type(UNDEFINED), set_size(size), set_values(new T[size]) {}

    Set(int size, T *set_values, SetType set_type);

    T &operator[](int index);

    T hamiltonian(Lattice<T> matrix);
};

template<typename T>
Set<T>::Set(int _size, T *_set_values, SetType _set_type) {
    set_size = _size;
    set_values = _set_values;
    set_type = _set_type;
}

template<typename T>
T &Set<T>::operator[](int index) {
    return set_values[index];
}

template<typename T>
T Set<T>::hamiltonian(Lattice<T> matrix) {
    T ham = 0;
    for (int i = 0; i < set_size; ++i)
        for (int j = 0; j < set_size; ++j)
            if (i == j)
                ham += matrix(i, j) * set_values[i];
            else
                ham += matrix(i, j) * set_values[i] * set_values[j];
    return ham;
}

#endif //MARS_CI_SET_H
