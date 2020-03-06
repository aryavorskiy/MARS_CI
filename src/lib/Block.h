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

template<typename T>
struct Block {
    typedef std::vector<int> set_link;
    T *block_values;
    set_link *links;
    int set_size;
    int set_count;

    Block() = default;

    Block(int set_size, int set_count, T *block_values, set_link *links) :
            set_size(set_size), set_count(set_count), block_values(block_values), links(links) {};

    BigFloat meanField(Lattice<T> matrix, int set_index, int spin_index, BigFloat interaction_multiplier);

    Set<T> operator[](int index);
};

template<typename T>
BigFloat Block<T>::meanField(Lattice<T> matrix, int set_index, int spin_index, BigFloat interaction_multiplier) {
    assert(matrix.size() == set_size);
    BigFloat mean_field{};

    // Set interaction in block
    for (int link_element : links[set_index]) {
        mean_field += interaction_multiplier * (float) (0.5 * logf(
                (1 + block_values[link_element * set_size + spin_index]) /
                (1 - block_values[link_element * set_size + spin_index])
        ));
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
