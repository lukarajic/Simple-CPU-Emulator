#include "Cache.hpp"
#include <cmath>

Cache::Cache(uint32_t num_sets, uint32_t block_size) 
    : num_sets(num_sets), block_size(block_size) {
    for (uint32_t i = 0; i < num_sets; ++i) {
        lines.emplace_back(block_size);
    }
}

uint32_t Cache::get_index(uint32_t address) const {
    // index bits = log2(num_sets)
    return (address / block_size) % num_sets;
}

uint32_t Cache::get_tag(uint32_t address) const {
    uint32_t block_offset_bits = static_cast<uint32_t>(std::log2(block_size));
    uint32_t index_bits = static_cast<uint32_t>(std::log2(num_sets));
    return address >> (block_offset_bits + index_bits);
}

bool Cache::access(uint32_t address, bool is_write) {
    (void)is_write; // Currently, we don't distinguish write-through/back for simplicity
    uint32_t index = get_index(address);
    uint32_t tag = get_tag(address);

    CacheLine& line = lines[index];

    if (line.valid && line.tag == tag) {
        hits++;
        return true;
    } else {
        misses++;
        // Simulate a line fill
        line.valid = true;
        line.tag = tag;
        return false;
    }
}
