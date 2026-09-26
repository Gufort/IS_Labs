#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>

using u64 = uint64_t;

class custom_set
{
public:
    explicit custom_set(size_t log2_cap = 22)
        : mask((size_t(1) << log2_cap) - 1),
          keys(mask + 1, 0),
          used(mask + 1, 0),
          count(0), tombstones(0) {}

    // 0 = пусто, 1 = занято, 2 = tombstone
    bool contains(u64 k) const {
        size_t i = hash(k) & mask;
        while (used[i]) {
            if (used[i] == 1 && keys[i] == k) return true;
            i = (i + 1) & mask;
        }
        return false;
    }

    // true — вставили, false — уже был
    bool insert(u64 k) {
        size_t i = hash(k) & mask;
        size_t first_tomb = SIZE_MAX;
        while (used[i]) {
            if (used[i] == 1) {
                if (keys[i] == k) return false;
            } else {
                if (first_tomb == SIZE_MAX) first_tomb = i;
            }
            i = (i + 1) & mask;
        }
        const size_t slot = (first_tomb != SIZE_MAX) ? first_tomb : i;
        if (first_tomb != SIZE_MAX) --tombstones;
        keys[slot] = k;
        used[slot] = 1;
        ++count;
        return true;
    }

    void erase(u64 k) {
        size_t i = hash(k) & mask;
        while (used[i]) {
            if (used[i] == 1 && keys[i] == k) {
                used[i] = 2;
                --count;
                ++tombstones;
                return;
            }
            i = (i + 1) & mask;
        }
    }

    void clear() {
        std::fill(used.begin(), used.end(), 0);
        count = 0;
        tombstones = 0;
    }

    size_t size() const { return count; }
    size_t tombs() const { return tombstones; }

private:
    size_t mask;
    std::vector<u64> keys;
    std::vector<uint8_t> used;
    size_t count;
    size_t tombstones;

    static inline u64 hash(u64 x) {
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return x;
    }
};