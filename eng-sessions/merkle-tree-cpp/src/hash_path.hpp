#pragma once

#include "sha256_hasher.hpp"
#include <vector>
#include <cstdint>
#include <utility>

/**
 * Mimics the TypeScript HashPath class.
 * A HashPath is a collection of pairs of 32-byte hashes, each pair
 * representing the left/right child at a given layer in the Merkle path.
 */
class HashPath {
public:
    // Each entry in 'data' is (left_node, right_node).
    // Each node is 32 bytes. We'll store them in a sha256_hash_t of length 32.
    std::vector<std::pair<sha256_hash_t, sha256_hash_t>> data;

    HashPath() = default;

    HashPath(const std::vector<std::pair<sha256_hash_t, sha256_hash_t>> &d)
        : data(d) {
    }

    /**
     * Flatten the data into a single buffer, each pair appended:
     *   first(32 bytes) + second(32 bytes)
     * repeated for each layer in the path.
     */
    std::vector<uint8_t> to_buffer() const
    {
        std::vector<uint8_t> buf;
        buf.reserve(data.size() * 64);
        for (const auto &pair_item : data) {
            buf.insert(buf.end(), pair_item.first.begin(), pair_item.first.end());
            buf.insert(buf.end(), pair_item.second.begin(), pair_item.second.end());
        }
        return buf;
    }

    /**
     * Construct a HashPath from a buffer created by 'to_buffer()'.
     * For each 64 bytes, the first 32 are left, the second 32 are right.
     */
    static HashPath from_buffer(const std::vector<uint8_t>& buf)
    {
        HashPath path;
        if (buf.size() % 64 != 0) {
            // Invalid. In real usage, might throw or handle differently.
            return path;
        }
        size_t count = buf.size() / 64;
        path.data.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            std::array<uint8_t, 32> left;
            std::array<uint8_t, 32> right;
            std::copy(buf.begin() + i * 64, buf.begin() + i * 64 + 32, left.begin());
            std::copy(buf.begin() + i * 64 + 32, buf.begin() + i * 64 + 64, right.begin());
            path.data.emplace_back(std::make_pair(left, right));
        }
        return path;
    }
};

/**
 * Simple equality operator for testing or comparison.
 */
inline bool operator==(const HashPath &a, const HashPath &b) {
    if (a.data.size() != b.data.size()) {
        return false;
    }
    for (size_t i = 0; i < a.data.size(); ++i) {
        if (a.data[i].first != b.data[i].first ||
            a.data[i].second != b.data[i].second) {
            return false;
        }
    }
    return true;
}
