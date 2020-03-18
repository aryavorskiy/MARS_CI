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
    typedef std::shared_ptr<Set<T>> LinkedSet;
private:
    int set_size = 0;
    T *set_values = nullptr;
    std::vector<LinkedSet> linked_sets{};
public:
    SetType set_type = EMPTY;

    /**
     * Default Set constructor.
     */
    Set() = default;

    /**
     * Set constructor.
     * @param size Spin quantity
     * @param set_values Spin value array pointer
     * @param set_type Type of set (see SetType docs)
     */
    Set(int size, T *set_values, SetType set_type);

    /**
     * Create a one-directional link to another Set object.
     * @param linked_set Set object to link to
     */
    void createLink(Set<T> &linked_set);

    /**
    * Get spin from specified index.
    * @param index Spin index
    * @return Spin value
    */
    T operator[](int index);

    /**
     * Set spin at specified index.
     * @param index Spin index
     * @param value Spin value
     */
    void setSpin(int index, T value);;

    /**
     * Calculate hamiltonian of spin system.
     * @param lattice Lattice describing spin interactions
     * @return Hamiltonian value
     */
    T hamiltonian(Lattice<T> lattice);

    /**
     * Calculates mean field value for specified spin.
     * @param lattice Lattice describing spin interactions
     * @return Mean field value
     */
    BigFloat meanField(int spin_index, Lattice<T> lattice, BigFloat interaction_multiplier);

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
T Set<T>::operator[](int index) {
    return set_values[index];
}

template<typename T>
void Set<T>::setSpin(int index, T value) {
    set_values[index] = value;
}

template<typename T>
T Set<T>::hamiltonian(Lattice<T> lattice) {
    T ham = 0;
    for (int i = 0; i < set_size; ++i)
        for (int j = i; j < set_size; ++j)
            if (i == j)
                ham += lattice(i, j) * set_values[i];
            else
                ham += lattice(i, j) * set_values[i] * set_values[j];
    return ham;
}

template<typename T>
BigFloat Set<T>::meanField(int spin_index, Lattice<T> lattice, BigFloat interaction_multiplier) {
    BigFloat mean_field{};

    // Set interaction in block
    if (interaction_multiplier != 0) {
        for (LinkedSet linked_set : linked_sets) {
            mean_field += interaction_multiplier * (float) (0.5 * logf(
                    (1 + (*linked_set)[spin_index]) /
                    (1 - (*linked_set)[spin_index])
            ));
        }
    }

    // Spin interaction in set
    for (int i = 0; i < set_size; ++i) {
        if (i != spin_index)
            mean_field += set_values[i] * lattice(i, spin_index);
    }
    return mean_field;
}

template<typename T>
void Set<T>::createLink(Set<T> &linked_set) {
    linked_sets.emplace_back(&linked_set);
}

template<typename T>
int Set<T>::size() { return set_size; }

#endif //MARS_CI_SET_H
