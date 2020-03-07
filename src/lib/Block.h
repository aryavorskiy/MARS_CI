//
// Created by aryavorskiy on 02.03.2020.
//

#ifndef MARS_CI_BLOCK_H
#define MARS_CI_BLOCK_H

#include <cassert>
#include <string>
#include <vector>
#include "Lattice.h"
#include "BigFloat.h"
#include "Set.h"

/**
 * Represents a union of several spin sets that interact with each other.
 * Set interaction is described by the links field.
 * @tparam T Spin value type
 */
template<typename T>
struct Block {
    /**
     * A set_link object contains indices of sets that interact with the given one.
     * The links object contains a set_link for every set in block.
     */
    typedef std::vector<int> set_link;

    int set_size = 0;
    int set_count = 0;
    T *block_values = nullptr;
    set_link *links = nullptr;

    /**
     * Default Block constructor
     */
    Block() = default;

    /**
     * Block constructor.
     * @param set_size Quantity of spins in sets
     * @param set_count Quantity of sets in block
     * @param block_values Spin values array pointer
     * @param links Link array pointer
     */
    Block(int set_size, int set_count, T *block_values, set_link *links) :
            set_size(set_size), set_count(set_count), block_values(block_values), links(links) {};

    /**
     * Calculates mean field value for specified spin in specified set
     * @param lattice Lattice describing spin interactions
     * @param set_index Index of set in block
     * @param spin_index Index of spin in set
     * @param interaction_multiplier Number describing set interaction
     * @return Mean field value
     */
    BigFloat meanField(Lattice<T> lattice, int set_index, int spin_index, BigFloat interaction_multiplier);

    /**
     * Get Set from specified index.
     * @param index Index of set in block
     * @return Set object
     */
    Set<T> operator[](int index);
};

template<typename T>
BigFloat Block<T>::meanField(Lattice<T> matrix, int set_index, int spin_index, BigFloat interaction_multiplier) {
    assert(matrix.size() == set_size);
    BigFloat mean_field{};

    // Set interaction in block
    if (interaction_multiplier != 0) {
        for (int link_element : links[set_index]) {
            mean_field += interaction_multiplier * (float) (0.5 * logf(
                    (1 + block_values[link_element * set_size + spin_index]) /
                    (1 - block_values[link_element * set_size + spin_index])
            ));
        }
    }

    // Spin interaction in set
    for (int i = 0; i < set_size; ++i) {
        if (i != spin_index)
            mean_field += block_values[set_index * set_size + i] * matrix(i, spin_index);
    }
    return mean_field;
}

template<typename T>
Set<T> Block<T>::operator[](int index) {
    SetType set_type;
    if (links[index].empty())
        set_type = INDEPENDENT;
    else if (links[index][0] == -1)
        set_type = NO_ANNEAL;
    else
        set_type = DEPENDENT;
    return Set<T>(set_size, block_values + index * set_size, set_type);
}


#endif //MARS_CI_BLOCK_H
