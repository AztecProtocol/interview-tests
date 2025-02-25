#pragma once

#include "hash_path.hpp"
#include "mock_db.hpp"
#include "sha256_hasher.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * The MerkleTree class implements a Merkle tree—a data structure that enables efficient
 * proofs of membership.
 *
 * NOTE: This is a placeholder version. All helper methods and internal implementation details
 * have been removed. Replace the "// Implement" comments with the appropriate logic.
 */
class MerkleTree {
  private:
    static constexpr uint32_t MAX_DEPTH = 32;
    static constexpr uint32_t LEAF_BYTES = 64;
  public:
    /**
     * Constructs a new or existing tree.
     *
     * @param db The underlying database.
     * @param name The name of the tree.
     * @param depth The tree’s depth (with leaves at layer = depth).
     * @param root (Optional) The pre-existing tree root.
     *
     * Throws std::runtime_error if depth is not in [1, 32].
     */
    MerkleTree(MockDB& db, const std::string& name, uint32_t depth, const sha256_hash_t& root = {})
        : db(db)
        , name(name)
        , depth(depth)
        , root(root)
        , hasher()
    {
        if (!(depth >= 1 && depth <= MAX_DEPTH)) {
            throw std::runtime_error("Bad depth");
        }
        // Implement.
    }

    /**
     * Creates (or restores) a MerkleTree instance.
     *
     * @param db The underlying database.
     * @param name The name of the tree.
     * @param depth The tree’s depth (default is 32).
     * @return A MerkleTree instance.
     */
    static MerkleTree create(MockDB& db, const std::string& name, uint32_t depth = MAX_DEPTH)
    {
        return MerkleTree(db, name, depth);
    }

    /**
     * Returns the current Merkle tree root (32 bytes).
     */
    sha256_hash_t get_root() const
    {
        return root;
    }

    /**
     * Returns the hash path (Merkle proof) for a particular leaf index.
     *
     * @param index The leaf index.
     * @return A HashPath object.
     */
    HashPath get_hash_path(uint64_t index) const
    {
        // Implement.
        return HashPath();
    }

    /**
     * Updates the leaf at the given index with the specified 64-byte value.
     *
     * @param index The index of the leaf.
     * @param value A 64-byte vector representing the leaf data.
     * @return The new 32-byte tree root.
     *
     * Throws std::runtime_error if value is not exactly 64 bytes.
     */
    sha256_hash_t update_element(uint64_t index, const std::vector<uint8_t>& value)
    {
        // Implement.
        return root;
    }

  private:
    // Core member variables.
    MockDB& db;
    std::string name;
    uint32_t depth;
    sha256_hash_t root;
    Sha256Hasher hasher;
};
