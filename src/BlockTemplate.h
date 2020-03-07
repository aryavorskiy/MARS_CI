//
// Created by alexander on 04.03.2020.
//

#ifndef MARS_CI_BLOCKTEMPLATE_H
#define MARS_CI_BLOCKTEMPLATE_H

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

#include "lib/Block.h"
#include "BlockTemplate.h"
#include "lib/Set.h"

template<typename T>
class BlockTemplate {
private:
    typedef std::vector<int> set_link;

    int set_size;
    int set_count;
    Set<T> *sets;
    set_link *links;

    void loadLinks(const std::string &link_filename);

public:
    BlockTemplate() : set_size(0), set_count(0), sets(nullptr), links(nullptr) {};

    BlockTemplate(int set_size, const std::string &block_filename, const std::string &link_filename);

    BlockTemplate(int set_size, int set_count, const std::string &link_filename);

    Block<T> instance();

    int size();
};

template<typename T>
struct RandomSet : Set<T> {
    RandomSet() = default;

    explicit RandomSet(int size) {
        this->set_size = size;
        this->set_values = new T[size];
    }

    T operator()(int) {
        return Random::uniform();
    }
};

template<typename T>
void BlockTemplate<T>::loadLinks(const std::string &link_filename) {
    links = new set_link[set_count];
    if (link_filename == "NONE") {
        for (int link_index = 0; link_index < set_count; ++link_index)
            links[link_index] = set_link();
        return;
    }
    auto ifs = std::ifstream(link_filename);
    int block_size = 0;
    ifs >> block_size;
    assert(block_size == set_count);
    std::string line;
    getline(ifs, line);
    for (int link_index = 0; link_index < block_size; ++link_index) {
        // Read the link file
        set_link link = set_link();
        getline(ifs, line);
        if (line.empty()) {
            // Ignore empty line
            link_index--;
        } else if (line == "EMPTY" || line == "NONE") {
            // Set interacts with no others
        } else if (line == "ALL") {
            // Set interacts with every other except himself
            for (int i = 0; i < block_size; ++i)
                if (i != link_index)
                    link.push_back(i);
        } else if (line == "NO_ANNEAL") {
            // Set values will be unchanged during block annealing
            link.push_back(-1);
        } else {
            // Read line to find out
            std::stringstream line_parser(line);
            for (int j = 0; j < block_size; ++j) {
                int buf = 0;
                line_parser >> buf;
                if (link.empty() || std::find(link.begin(), link.end(), buf) != link.end())
                    // Duplicates are ignored
                    link.push_back(buf);
            }
        }
        links[link_index] = link;
    }
}

template<typename T>
BlockTemplate<T>::BlockTemplate(int _set_size, const std::string &block_filename, const std::string &link_filename):
        set_size(_set_size) {
    auto ifs = std::ifstream(block_filename);
    ifs >> set_count;
    sets = new Set<T>[set_count]{Set<T>(set_size)};

    std::string line;
    getline(ifs, line);
    for (int set_index = 0; set_index < set_count; ++set_index) {
        // Read the block file
        getline(ifs, line);
        if (line.empty()) {
            // Ignore empty line
            set_index--;
        } else if (line == "RAND") {
            // Random line
            sets[set_index] = RandomSet<T>(set_size);
        } else {
            // Read line from line buffer
            auto line_parser = std::istringstream(line);
            for (int i = 0; i < set_size; ++i) {
                T buf;
                line_parser >> buf;
                sets[set_index][i] = buf;
            }
        }
    }
    // Initialize links
    loadLinks(link_filename);
}

template<typename T>
BlockTemplate<T>::BlockTemplate(int set_size, int set_count, const std::string &link_filename) :
        set_size(set_size), set_count(set_count) {
    sets = new RandomSet<T>[set_count];
    for (int set_index = 0; set_index < set_count; ++set_index)
        sets[set_index] = RandomSet<T>(set_size);

    // Initialize links
    loadLinks(link_filename);
}

template<typename T>
Block<T> BlockTemplate<T>::instance() {
    T *block_data = new T[set_count * set_size];
    for (int set_index = 0; set_index < set_count; ++set_index) {
        for (int spin_index = 0; spin_index < set_size; ++spin_index) {
            block_data[set_index * set_size + spin_index] = sets[set_index](spin_index);
        }
    }
    return Block<T>(set_size, set_count, block_data, links);
}

template<typename T>
int BlockTemplate<T>::size() {
    return set_count;
}


#endif //MARS_CI_BLOCKTEMPLATE_H
