#include "indexed_merkle_tree.hpp"
#include <stdlib/merkle_tree/hash.hpp>

namespace plonk {
namespace stdlib {
namespace indexed_merkle_tree {

/**
 * Initialise an indexed merkle tree state with all the leaf values: H({0, 0, 0}).
 * Note that the leaf pre-image vector `leaves_` must be filled with {0, 0, 0} only at index 0.
 */
IndexedMerkleTree::IndexedMerkleTree(size_t depth)
    : depth_(depth)
{
    ASSERT(depth_ >= 1 && depth <= 32);
    total_size_ = 1UL << depth_;
    hashes_.resize(total_size_ * 2 - 2);

    // Exercise: Build the initial state of the entire tree.
}

/**
 * Fetches a hash-path from a given index in the tree.
 * Note that the size of the fr_hash_path vector should be equal to the depth of the tree.
 */
fr_hash_path IndexedMerkleTree::get_hash_path(size_t)
{
    // Exercise: fill the hash path for a given index.
    fr_hash_path path(depth_);
    return path;
}

/**
 * Update the node values (i.e. `hashes_`) given the leaf hash `value` and its index `index`.
 * Note that indexing in the tree starts from 0.
 * This function should return the updated root of the tree.
 */
fr IndexedMerkleTree::update_element_internal(size_t, fr const&)
{
    // Exercise: insert the leaf hash `value` at `index`.
    return 0;
}

/**
 * Insert a new `value` in a new leaf in the `leaves_` vector in the form: {value, nextIdx, nextVal}
 * You will need to compute `nextIdx, nextVal` according to the way indexed merkle trees work.
 * Further, you will need to update one old leaf pre-image on inserting a new leaf.
 * Lastly, insert the new leaf hash in the tree as well as update the existing leaf hash of the old leaf.
 */
fr IndexedMerkleTree::update_element(fr const&)
{
    // Exercise: add a new leaf with value `value` to the tree.
    return 0;
}

} // namespace indexed_merkle_tree
} // namespace stdlib
} // namespace plonk