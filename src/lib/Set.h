//
// Created by alexander on 03.03.2020.
//

#ifndef MARS_CI_SET_H
#define MARS_CI_SET_H

#include "Lattice.h"

enum SetType {
    INDEPENDENT,    // Does not interact with other sets
    DEPENDENT,      // Interacts with some of other sets
    NO_ANNEAL,      // Is not annealed during the process
    UNDEFINED,      // Not defined because no link info given
    EMPTY           // Spin value array memory is not allocated yet
};

/**
 * Represents a set of spins.
 * @tparam T Spin value type
 */
template<typename T>
class Set {
protected:
    int set_size;
    T *set_values;
public:
    SetType set_type;

    /**
     * Default Set constructor.
     */
    Set() : set_size(0), set_values(nullptr), set_type(EMPTY) {}

    /**
     * Set constructor.
     * @param size Spin quantity
     * @param set_values Spin value array pointer
     * @param set_type Type of set (see SetType docs)
     */
    Set(int size, T *set_values, SetType set_type);

    /**
    * Get spin from specified index.
    * @param index Spin index
    * @return Spin value reference
    */
    T &operator[](int index);

    /**
     * Calculate hamiltonian of spin system.
     * @param lattice Lattice describing spin interactions
     * @return Hamiltonian value
     */
    T hamiltonian(Lattice<T> lattice);

    /**
     * Get spin count in set.
     * @return Spin count
     */
    int size();
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
T Set<T>::hamiltonian(Lattice<T> lattice) {
    T ham = 0;
    for (int i = 0; i < set_size; ++i)
        for (int j = 0; j < set_size; ++j)
            if (i == j)
                ham += lattice(i, j) * set_values[i];
            else
                ham += lattice(i, j) * set_values[i] * set_values[j];
    return ham;
}

template<typename T>
int Set<T>::size() { return set_size; }

#endif //MARS_CI_SET_H
