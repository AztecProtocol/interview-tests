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

## generateProof function usagement.

```bash
let indexzeroexpectation = new HashPath([[e01], [e11]]);
console.log('a', indexzeroexpectation);
console.log('b', await treee.generateProof(0));

HashPath {
      data: [
        [
          <Buffer 16 ab ab 34 1f b7 f3 70 e2 7e 4d ad cf 81 76 6d d0 df d0 ae 64 46 94 77 bb 2c f6 61 49 38 b2 af>
        ],
        [
          <Buffer 56 4a 42 6c 73 ba 2d ca 7b c3 67 df 3f 29 73 7e 94 8f ae c9 60 b8 d0 a1 5b 26 a5 0d 84 b3 15 67>
        ]
      ]
    }

HashPath {
      data: [
        [
          <Buffer 16 ab ab 34 1f b7 f3 70 e2 7e 4d ad cf 81 76 6d d0 df d0 ae 64 46 94 77 bb 2c f6 61 49 38 b2 af>
        ],
        [
          <Buffer 56 4a 42 6c 73 ba 2d ca 7b c3 67 df 3f 29 73 7e 94 8f ae c9 60 b8 d0 a1 5b 26 a5 0d 84 b3 15 67>
        ]
      ]
    }


let indexoneexpectation = new HashPath([[e00], [e11]]);
console.log('a', indexoneexpectation);
console.log('b', await treee.generateProof(1));

HashPath {
    data: [
    [
        <Buffer f5 a5 fd 42 d1 6a 20 30 27 98 ef 6e d3 09 97 9b 43 00 3d 23 20 d9 f0 e8 ea 98 31 a9 27 59 fb 4b>
    ],
    [
        <Buffer 56 4a 42 6c 73 ba 2d ca 7b c3 67 df 3f 29 73 7e 94 8f ae c9 60 b8 d0 a1 5b 26 a5 0d 84 b3 15 67>
    ]
    ]
}

HashPath {
    data: [
    [
        <Buffer f5 a5 fd 42 d1 6a 20 30 27 98 ef 6e d3 09 97 9b 43 00 3d 23 20 d9 f0 e8 ea 98 31 a9 27 59 fb 4b>
    ],
    [
        <Buffer 56 4a 42 6c 73 ba 2d ca 7b c3 67 df 3f 29 73 7e 94 8f ae c9 60 b8 d0 a1 5b 26 a5 0d 84 b3 15 67>
    ]
    ]
}

let indextwoexpectation = new HashPath([[e03], [e10]]);
console.log('xxx', await treee.generateProof(2));
console.log('jasgg', indextwoexpectation);

HashPath {
    data: [
    [
        <Buffer e7 b4 bb 67 55 1d de 95 89 c1 55 3d fd a3 7a 94 2a 18 ca f1 84 f9 cc 16 29 d2 5c f5 c6 0b e4 16>
    ],
    [
        <Buffer 0f b6 10 eb 85 69 bb 12 e0 76 87 ea 9d 70 25 e3 ef ae 87 0a f0 a0 9a 36 cd fb 22 2d ff 79 50 b3>
    ]
    ]
}

HashPath {
    data: [
    [
        <Buffer e7 b4 bb 67 55 1d de 95 89 c1 55 3d fd a3 7a 94 2a 18 ca f1 84 f9 cc 16 29 d2 5c f5 c6 0b e4 16>
    ],
    [
        <Buffer 0f b6 10 eb 85 69 bb 12 e0 76 87 ea 9d 70 25 e3 ef ae 87 0a f0 a0 9a 36 cd fb 22 2d ff 79 50 b3>
    ]
    ]
}

expect(await treee.generateProof(0)).toEqual(indexzeroexpectation);
```

You can verify from image for example why index 0 (e00) need e01 and e11 because with this two hash your reach root.

for index 1 (e01) need e00 and e11 because with this two hash your reach root.

for index 2 (e02) need e03 and e10 because with this two hash your reach root. at this point e11 changes as e10 because index 2 (e02) is the right at depth 2.

![Merkle Tree](https://github.com/omgbbqhaxx/interview-tests/blob/master/eng-sessions/merkle-tree/src/proof.png)

# Important Resources

- https://hackmd.io/@kullervo/commitmentVector
- https://medium.com/@chaisomsri96/statelessness-series-part4-exploring-the-verkle-trie-structure-d97a8c85363e
- https://docs.aztec.network/protocol-specs/state/tree-implementations
- https://hackmd.io/@aztec-network/ryJ8wxfKK
