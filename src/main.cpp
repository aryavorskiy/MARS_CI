#include <iostream>
#include <cmath>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>

#include "lib/BigFloat.h"
#include "lib/Lattice.h"
#include "BlockTemplate.h"
#include "AnnealingRun.h"

#define VERSION "3.2"
#define BUILD 1
#define NO_INPUT

/*
 * TERMINOLOGY:
 * Mat - describes J_ij lattice
 * Set - describes system mean spin values at certain state
 * Block - several sets which descend simultaneously and interact with each other
 * Link - A std::std::vector describing set interaction in a block
 */

template<typename T>
std::ostream &operator<<(std::ostream &out, AnnealingRun<T> run) {
    for (int set_index = 0; set_index < run.block.set_count; ++set_index) {
        out << "Set #" << set_index << "; ";
        switch (run.block[set_index].set_type) {
            case INDEPENDENT:
                out << "Type: Independent; ";
                break;
            case DEPENDENT:
                out << "Type: Dependent; ";
                break;
            case NO_ANNEAL:
                out << "Type: No_anneal; ";
                break;
        }
        out << "Hamiltonian: " << run.block[set_index].hamiltonian(run.matrix) << "; Data:" << std::endl;
        for (int spin_index = 0; spin_index < run.block.set_size; ++spin_index)
            out << run.block[set_index][spin_index] << " ";
        out << std::endl;
    }
    return out;
}

template<typename T>
void anneal_output_silent(AnnealingRun<T> run) {
    float start_temp = run.temperature;
    run.anneal();
    std::cout << start_temp;
    for (int set_index = 0; set_index < run.block.set_count; ++set_index) {
        switch (run[set_index].set_type) {
            case INDEPENDENT:
                std::cout << " <" << run[set_index].hamiltonian(run.matrix) << ">";
                break;
            case NO_ANNEAL:
                std::cout << " (" << run[set_index].hamiltonian(run.matrix) << ")";
                break;
            default:
                std::cout << " " << run[set_index].hamiltonian(run.matrix);
                break;
        }
    }
    std::cout << std::endl;
}

template<typename T>
void anneal_output(AnnealingRun<T> run, std::string results_filename) {
    std::ofstream file_stream = std::ofstream(results_filename);
    float start_temp = run.temperature;
    file_stream << "Started processing block from temperature " << start_temp << ":" << std::endl;
    file_stream << run << std::endl;
    anneal_output_silent(run);
    file_stream << "Finished processing block; Start temperature was " << start_temp << "; Took " << run.step_counter
                << " steps; Block data:" << std::endl;
    file_stream << run << std::endl;
}

int main() {
    typedef float value_type;
    std::cout << "MARS analysis by A. Yavorski, CPU edition, version " << VERSION << ", build " << BUILD << std::endl;

    // Load temperature bounds
    float temp_start = 10, temp_final = 10, annealing_step = 0.01;
#ifndef NO_INPUT
    std::cout << "Start temp?" << std::endl;
    std::cin >> temp_start;
    std::cout << "Final temp?" << std::endl;
    std::cin >> temp_final;
    std::cout << "Annealing step?" << std::endl;
    std::cin >> annealing_step;
#endif

    // Load lattice
    Lattice<value_type> lattice;
    std::string lattice_initializer = "100";
#ifndef NO_INPUT
    std::cout << "Lattice file path (or size if random matrix needed)?" << std::endl;
    std::cin >> lattice_initializer;
#endif

    try {
        // User entered size
        lattice = Lattice<value_type>(std::stoi(lattice_initializer), true);
    }
    catch (std::exception &e) {
        // User entered path
        lattice = Lattice<value_type>(lattice_initializer);
    }

    // Load thread quantity
    int threads = 1;
#ifndef NO_INPUT
    std::cout << "Thread quantity?" << std::endl;
    std::cin >> threads;
#endif

    // Load block
    std::string block_filename = "1";
    int block_count = 1;
#ifndef NO_INPUT
    std::cout << "Block file location (Enter block size to create a random block)?" << std::endl;
    std::cin >> block_filename;
    std::cout << "Block quantity?" << std::endl;
    std::cin >> block_count;
#endif

    // Load link configuration
    std::string links_filename = "NONE";
#ifndef NO_INPUT
    std::cout << "Links file location (NONE for no interaction)?" << std::endl;
    std::cin >> links_filename;
#endif

    BlockTemplate<value_type> block_template;
    try {
        int block_size = stoi(block_filename);
        block_template = BlockTemplate<value_type>(lattice.size(), block_size, links_filename);
    } catch (std::exception &e) {
        block_template = BlockTemplate<value_type>(lattice.size(), block_filename, links_filename);
    }

    // Interaction multiplier
    float mul_log = 0;
#ifndef NO_INPUT
    std::cout << "Interaction multiplier (decimal log)?" << std::endl;
    std::cin >> mul_log;
#endif

    BigFloat interaction_multiplier = BigFloat(exp10f(mul_log - (int) mul_log), (int) mul_log);

    float temp_interaction_threshold = 1;
#ifndef NO_INPUT
    std::cout << "Temperature threshold?" << std::endl;
    std::cin >> temp_interaction_threshold;
#endif

    // Enable/disable full log
    std::string results_filename = "NONE";
#ifndef NO_INPUT
    std::cout << "File to save all results (NONE for no saving)?" << std::endl;
    std::cin >> results_filename;
#endif

    // Start annealing
    bool *thread_flags = new bool[threads]{true};
    int threads_launched = 0;
    while (threads_launched < block_count)
        for (int thrIndex = 0; thrIndex < threads; thrIndex++)
            if (thread_flags[thrIndex] and threads_launched < block_count) {  // Free place for thread detected
                thread_flags[thrIndex] = false;
                AnnealingRun<value_type> run = AnnealingRun<value_type>(std::ref(lattice));
                run.block = block_template.instance();
                run.temperature =
                        temp_start + ((float) threads_launched / (float) block_count) * (temp_final - temp_start);
                run.temperature_step = annealing_step;
                run.interaction_multiplier = interaction_multiplier;

                // Launch new run on a separate thread
                if (results_filename == "NONE")
                    std::thread(anneal_output_silent<value_type>, run).detach();
                else {
                    std::thread(anneal_output<value_type>, run, results_filename).detach();
                }
                threads_launched++;
            }

    bool any_threads_running = true;
    while (any_threads_running) {
        any_threads_running = false;
        for (int i = 0; i < threads; i++)
            any_threads_running = (any_threads_running || !thread_flags[i]);
    }
}
