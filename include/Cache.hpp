#ifndef CACHE_HPP
#define CACHE_HPP

#include <cstdint>
#include <vector>
#include <iostream>

struct CacheLine {
    bool valid = false;
    uint32_t tag = 0;
    std::vector<uint8_t> data;

    CacheLine(uint32_t block_size) : data(block_size, 0) {}
};

class Cache {
public:
    Cache(uint32_t num_sets, uint32_t block_size);

    // Returns true if hit, false if miss
    bool access(uint32_t address, bool is_write);

    uint32_t get_hits() const { return hits; }
    uint32_t get_misses() const { return misses; }

private:
    uint32_t num_sets;
    uint32_t block_size;
    std::vector<CacheLine> lines;

    uint32_t hits = 0;
    uint32_t misses = 0;

    // Address decomposition helpers
    uint32_t get_tag(uint32_t address) const;
    uint32_t get_index(uint32_t address) const;
};

#endif // CACHE_HPP
