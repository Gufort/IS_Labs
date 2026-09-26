// U64Map.h
#pragma once
#include <cstdint>
#include <vector>

using u64 = uint64_t;

class custom_map {
public:
    explicit custom_map(size_t capacity = 22)
        : mask((1ULL << capacity) - 1),
          keys(mask + 1, 0),
          vals(mask + 1, 0),
          used(mask + 1, 0) {}

    // nullptr — нет ключа
    const int* find(u64 k) const {
        size_t i = hash(k) & mask;
        while (used[i]) {
            if (keys[i] == k) return &vals[i];
            i = (i + 1) & mask;
        }
        return nullptr;
    }

    void insert_or_assign(u64 k, int v) {
        size_t i = hash(k) & mask;
        while (used[i]) {
            if (keys[i] == k) { vals[i] = v; return; }
            i = (i + 1) & mask;
        }
        keys[i] = k;
        vals[i] = v;
        used[i] = 1;
    }

    void clear() {
        std::fill(used.begin(), used.end(), 0);
    }

private:
    static inline u64 hash(u64 x) {
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return x;
    }

    size_t mask;
    std::vector<u64> keys;
    std::vector<int> vals;
    std::vector<uint8_t> used;
};