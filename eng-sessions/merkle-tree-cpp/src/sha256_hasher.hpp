#pragma once

#include "sha256.hpp"
#include <cstdint>
#include <string>
#include <vector>

using sha256_hash_t = std::array<uint8_t, 32>;

/**
 * Interface:
 *  - compress(lhs, rhs): concatenates lhs and rhs (64 bytes total) and returns their 32-byte SHA256 hash.
 *  - hash(data): returns a 32-byte SHA256 hash of arbitrary-length data.
 */
class Sha256Hasher {
  public:
    /**
     * Given two 32-byte buffers, return a 32-byte digest representing their concatenation.
     */
    std::array<uint8_t, 32> compress(const sha256_hash_t& lhs, const sha256_hash_t& rhs)
    {
        // They should each be 32 bytes for a merkle node. We'll concat them and compute the SHA-256 hash.
        SHA256 sha;
        sha.update(lhs.data(), lhs.size());
        sha.update(rhs.data(), rhs.size());
        return sha.digest();
    }

    /**
     * Given data of arbitrary length, return its 32-byte SHA256 hash.
     */
    std::array<uint8_t, 32> hash(const std::vector<uint8_t>& data)
    {
        SHA256 sha;
        sha.update(data.data(), data.size());
        return sha.digest();
    }
};
