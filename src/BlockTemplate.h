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
#include "SetTemplate.h"
#include "lib/Set.h"

/**
 * Represents a structure that can make a block that matches a given template.
 * The BlockTemplate<T>::instance method creates a new instance of a Block object.
 * @tparam T Block spin value type
 */
template<typename T>
class BlockTemplate {
private:
    typedef std::vector<int> SetLink;
    typedef std::unique_ptr<SetTemplate<T>> SetReference;

    int set_size = 0;
    int set_count = 0;
    std::vector<SetReference> sets = std::vector<SetReference>();
    SetLink *links = nullptr;

    /**
     * Load link information from specified file.
     * @param link_filename Filename where link data is stored
     */
    void loadLinks(const std::string &link_filename);

public:
    /**
     * Default BlockTemplate constructor.
     */
    BlockTemplate() = default;

    /**
     * BlockTemplate constructor that creates an empty block.
     * @param set_size Size of sets in block
     * @param set_count Quantity of Sets in block
     * @param link_filename Filename where link data is stored
     */
    BlockTemplate(int set_size, int set_count, const std::string &link_filename);

    /**
     * BlockTemplate constructor that loads data from specified files.
     * @param set_size Size of sets in block
     * @param block_filename Filename where block template data is stored
     * @param link_filename Filename where link data is stored
     */
    BlockTemplate(int set_size, const std::string &block_filename, const std::string &link_filename);

    /**
     * Create a Block object that matches the template represented by this.
     * @return Block object
     */
    Block<T> instance();
};

template<typename T>
void BlockTemplate<T>::loadLinks(const std::string &link_filename) {
    links = new SetLink[set_count];
    if (link_filename == "NONE") {
        for (int link_index = 0; link_index < set_count; ++link_index)
            links[link_index] = SetLink();
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
        SetLink link = SetLink();
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
                if (link.empty() || std::find(link.begin(), link.end(), buf) == link.end())
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
    sets = std::vector<SetReference>();

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
            sets.emplace_back(new RandomSetTemplate<T>(set_size));
        } else {
            // Read line from line buffer
            sets.emplace_back(new GivenSetTemplate<T>(set_size, line));
        }
    }
    // Initialize links
    loadLinks(link_filename);
}

template<typename T>
BlockTemplate<T>::BlockTemplate(int set_size, int set_count, const std::string &link_filename) :
        set_size(set_size), set_count(set_count) {
    sets = std::vector<SetReference>();
    for (int set_index = 0; set_index < set_count; ++set_index)
        sets.emplace_back(new RandomSetTemplate<T>(set_size));

    // Initialize links
    loadLinks(link_filename);
}

template<typename T>
Block<T> BlockTemplate<T>::instance() {
    auto *sets_out = new Set<T>[set_count];
    for (int set_index = 0; set_index < set_count; ++set_index)
        sets_out[set_index] = sets[set_index]->instance();
    return Block<T>(set_count, sets_out, links);
}

#endif //MARS_CI_BLOCKTEMPLATE_H
