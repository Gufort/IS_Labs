#pragma once
#include <cstdint>
#include <vector>

using u64 = uint64_t;

class custom_set
{
public:
    custom_set(size_t capacity = 22)
    : mask((1ULL << capacity) - 1),
      keys(mask + 1, 0),
      used(mask + 1, 0),
      count(0) {}

    bool contains(uint64_t k) const {
        size_t i = hash(k) & mask;
        while (used[i]) {
            if (keys[i] == k) return true;
            i = (i + 1) & mask;
        }
        return false;
    }

    bool insert(uint64_t k) {
        size_t i = hash(k) & mask;
        while (used[i]) {
            if (keys[i] == k) return false;
            i = (i + 1) & mask;
        }
        keys[i] = k;
        used[i] = 1;
        ++count;
        return true;
    }
    void clear() {
        std::fill(used.begin(), used.end(), 0);
        count = 0;
    }

    size_t size() const { return count; }

private:
    size_t mask; //
    std::vector<u64> keys; // массив ключей
    std::vector<u64> used; // массив флаго занято/свободно
    size_t count; // количество ключей

    static inline u64 hash(uint64_t x) {
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return x;
    }
};
