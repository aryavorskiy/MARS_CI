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
    std::vector<BigFloat> probabilities{}, inv_probabilities{};

    BigFloat interactionMeanField(int spin_index, BigFloat interaction_multiplier);

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
     * Recalculate all stored equality probability values.
     * Call every time before performing a sweep on all spins
     */
    void recalculateProbabilities();

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
    for (unsigned int link_index = 0; link_index < linked_sets.size(); ++link_index) {
        probabilities[link_index] *=
                (1 + (*linked_sets[link_index])[index] * value) /
                (1 + (*linked_sets[link_index])[index] * set_values[index]);
        inv_probabilities[link_index] *=
                (1 - (*linked_sets[link_index])[index] * value) /
                (1 - (*linked_sets[link_index])[index] * set_values[index]);
    }
    set_values[index] = value;
}

template<typename T>
void Set<T>::recalculateProbabilities() {
    for (unsigned int link_index = 0; link_index < linked_sets.size(); ++link_index) {
        BigFloat prob{1}, inv_prob{1};
        for (int spin_index = 0; spin_index < set_size; ++spin_index) {
            prob *= (1 + (*linked_sets[link_index])[spin_index] * set_values[spin_index]) / 2.;
            inv_prob *= (1 - (*linked_sets[link_index])[spin_index] * set_values[spin_index]) / 2;
        }
        probabilities[link_index] = prob;
        inv_probabilities[link_index] = inv_prob;
    }
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
BigFloat Set<T>::interactionMeanField(int spin_index, BigFloat interaction_multiplier) {
    BigFloat interaction_mean_field{0};
    for (unsigned int link_index = 0; link_index < linked_sets.size(); ++link_index) {
        interaction_mean_field += interaction_multiplier * (0.5 * (
                (probabilities[link_index] * (1 + (*linked_sets[link_index])[spin_index]) /
                 (1 + (*linked_sets[link_index])[spin_index] * set_values[spin_index]) +
                 inv_probabilities[link_index] * (1 - (*linked_sets[link_index])[spin_index]) /
                 (1 - (*linked_sets[link_index])[spin_index] * set_values[spin_index])) /
                //-----------------------------------------------------------------------
                (probabilities[link_index] * (1 - (*linked_sets[link_index])[spin_index]) /
                 (1 + (*linked_sets[link_index])[spin_index] * set_values[spin_index]) +
                 inv_probabilities[link_index] * (1 + (*linked_sets[link_index])[spin_index]) /
                 (1 - (*linked_sets[link_index])[spin_index] * set_values[spin_index]))
        ).log());
    }
    return interaction_mean_field;
}

template<typename T>
BigFloat Set<T>::meanField(int spin_index, Lattice<T> lattice, BigFloat interaction_multiplier) {
    BigFloat interaction_mean_field =
            interaction_multiplier == 0 ? 0 : interactionMeanField(spin_index, interaction_multiplier);

    // Spin interaction in set
    double spin_mean_field = 0;
    for (int i = 0; i < set_size; ++i) {
        if (i != spin_index)
            spin_mean_field += set_values[i] * lattice(i, spin_index);
    }
    return interaction_mean_field + BigFloat(spin_mean_field);
}

template<typename T>
void Set<T>::createLink(Set<T> &linked_set) {
    linked_sets.emplace_back(&linked_set);
    probabilities.push_back(BigFloat{1});
    inv_probabilities.push_back(BigFloat{1});
}

template<typename T>
int Set<T>::size() { return set_size; }

#endif //MARS_CI_SET_H
