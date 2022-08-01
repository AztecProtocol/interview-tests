# Aztec Technical Challenge

The test provides you an opportunity to demonstrate the following:

- Your ability to write a data structure algorithm (in this case a merkle tree).
- Your ability to write clean, idiomatic TypeScript.

## Rationale

A core data structure in the Aztec system is the merkle tree. It's a simple binary tree structure where the root node is represented by the hash of its two child hashes. Given any set of data in the leaves, this leads to a unique root. Furthermore, proof of existence of a piece of data can be represented by a hash path, a list of pairwise child hashes at each layer, from leaf to root. Aztec stores all of its notes in such data structures, and when proofs are generated they use hash paths to prove the data they are modifying exists.

In this test you will be working on an implementation of a merkle tree.

## Merkle Tree Structure

- The merkle tree is of depth `32`, and is fully formed with leaves consisting of `64` zero bytes at every index.
- When inserting an element of arbitrary length, the value must first be `hash`ed to `32` bytes using sha256.
- Each node of the tree is computed by `compress`ing its left and right subtree hashes and taking the resulting sha256 hash.
- For reference, an unpopulated merkle tree will have a root hash of `1c9a7e5ff1cf48b4ad1582d3f4e4a1004f3b20d8c5a2b71387a4254ad933ebc5`.

The merkle tree is to be persisted in a key value store. `LevelUp` provides the basic key value store interface.

## Building and Running

After cloning the repo:

```bash
yarn install

# To run all tests.
yarn test

# To run tests, watching for changes.
yarn test --watch
```
