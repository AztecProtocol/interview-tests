# Aztec Technical Challenge

**WARNING: Do not fork this repository or make a public repository containing your solution. Either copy it to a private repository or submit your solution via other means.**

Links to solutions may be sent to hello@aztecprotocol.com.

Welcome, candidate. You have been selected to partake in the Aztec technical challenge. This source code repository contains the remains of a fully functional merkle tree hash path server. Select portions of the codebase have been removed, and it is your job to re-code the system to working operation.

The test provides you an opportunity to demonstrate the following:

- Your ability to write a data structure algorithm (in this case a merkle tree).
- Your ability to write an http service of production quality, with all considerations that entails.
- Your ability to write clean, idiomatic TypeScript.

Please include with your submission, a section at the end of this document, detailing any theoretical optimisations that could be made to the algorithm to improve its performance.

## Rationale

A core data structure in the Aztec system is the merkle tree. It's a simple binary tree structure where the root node is represented by the hash of its two child hashes. Given any set of data in the leaves, this leads to a unique root. Furthermore, proof of existence of a piece of data can be represented by a hash path, a list of pairwise child hashes at each layer, from leaf to root. Aztec stores all of its notes in such data structures, and when proofs are generated they use hash paths to prove the data they are modifying exists.

In this test you will be working on a service that maintains such a tree, and services hash path requests.

## Service Specification

The service itself is a `HashPathSource`.

```typescript
export interface TreeState {
  root: Buffer;
  size: number;
}

export interface HashPathSource {
  getTreeState(): Promise<TreeState>;
  getHashPath(index: number): Promise<HashPath>;
}
```

Internally, the service is listening to a `BlockSource`. An implementation that allows simulating the arrival of blocks is included in `LocalBlockSource`.

```typescript
export interface Block {
  id: number;
  created: Date;
  dataStartIndex: number;
  leafData: Buffer[];
}

export interface BlockSource {
  /**
   * Returns up to 5 blocks from block id `from`.
   */
  getBlocks(from: number): Promise<Block[]>;

  /**
   * Starts emitting blocks from `fromBlock`.
   */
  start(fromBlock?: number): void;

  /**
   * Stops emitting blocks.
   */
  stop(): void;

  on(event: 'block', fn: (block: Block) => void): void;

  removeAllListeners(): void;
}
```

Internally, the service is communicating with a `WorldStateDb`.

```typescript
export interface WorldStateDb {
  getRoot(): Buffer;
  getSize(): number;
  get(index: number): Promise<Buffer>;
  getHashPath(index: number): Promise<HashPath>;
  put(index: number, value: Buffer): Promise<Buffer>;
}
```

It is the job of the server to remain in sync with, and construct a merkle tree from the `BlockSource`, allowing for any client of a `HashPathSource` to query for and receive a hash path at a given index.

The `leafData` in a `Block` is to be inserted incrementally into the tree starting at `dataStartIndex`.

Pay attention to any comments in code for further implementation details.

## Client Considerations

Any client application should be able to interact with the server via a `HashPathSource`.

## Merkle Tree Structure

- The merkle tree is of depth `32`, and is fully pre-populated with leaves consisting of `64` zero bytes at every index.
- When inserting an element of arbitrary length, the value must first (internally) be compressed to `32` bytes using sha256.
- Each node of the tree is computed by concatenating its left and right subtree hashes and taking the resulting sha256 hash.
- For reference, an unpopulated merkle tree will have a root hash of `1c9a7e5ff1cf48b4ad1582d3f4e4a1004f3b20d8c5a2b71387a4254ad933ebc5`.
- The `size` of a merkle tree is always determined by its highest index element.

The merkle tree is provided a `Hasher`.

```typescript
export interface Hasher {
  compress(lhs: Buffer, rhs: Buffer): Buffer;
  hash(data: Buffer): Buffer;
}
```

## Time Considerations

It's expected you put in at about a days effort. It's not expected for you to complete all of it, and we will continue working on your solution in the code pairing session. Emphasis is on good quality tested code.

Prioritise the merkle tree implementation over the server implementation, but if there's anything you don't have time to implement, please add a section at the end of this document detailing any considerations you would make when implementing.

## Advanced Mode

If you found the above easy and want to go the extra mile, an implementation of a `WorldStateDb` has been provided (`ExternalWorldStateDb`) which will communicate with an external binary over stdin/stdout. This will allow you to demonstrate some skills around serializing data and systems level programming. `C++` is our primary systems level language today, so is preferred, but `Rust` will also be accepted.
