import { LevelUp, LevelUpChain } from 'levelup';
import { HashPath } from './hash_path';
import { Sha256Hasher } from './sha256_hasher';

const MAX_DEPTH = 32;
const LEAF_BYTES = 64; // All leaf values are 64 bytes.

/**
 * The merkle tree, in summary, is a data structure with a number of indexable elements, and the property
 * that it is possible to provide a succinct proof (HashPath) that a given piece of data, exists at a certain index,
 * for a given merkle tree root.
 */
export class MerkleTree {
  private hasher = new Sha256Hasher();
  private root = Buffer.alloc(32);

  /**
   * Constructs a new MerkleTree instance, either initializing an empty tree, or restoring pre-existing state values.
   * Use the async static `new` function to construct.
   *
   * @param db Underlying leveldb.
   * @param name Name of the tree, to be used when restoring/persisting state.
   * @param depth The depth of the tree, to be no greater than MAX_DEPTH.
   * @param root When restoring, you need to provide the root.
   */
  constructor(private db: LevelUp, private name: string, private depth: number, root?: Buffer) {
    if (!(depth >= 1 && depth <= MAX_DEPTH)) {
      throw Error('Bad depth');
    }

    // Minimum 2 depth required..
    if (!root) {
      console.log('constructor');
      let emptyroothash = this.hasher.hash(Buffer.alloc(LEAF_BYTES));
      //console.log('hash', hash);

      //should have correct empty tree root for depth 32 thirty two
      //here for first test pass.
      for (let i = 0; i < depth; i++) {
        const parent = this.hasher.compress(emptyroothash, emptyroothash);
        this.db.put(parent, this.hasher.concat([emptyroothash, emptyroothash]));
        emptyroothash = parent;
      }

      //console.log('hash2x', hash);
      this.root = emptyroothash;
    } else {
      this.root = root;
    }
  }

  /**
   * Constructs or restores a new MerkleTree instance with the given `name` and `depth`.
   * The `db` contains the tree data.
   */

  static async new(db: LevelUp, name: string, depth = MAX_DEPTH) {
    try {
      const metadata = await db.get(Buffer.from(name));
      const rootHash = metadata.slice(0, 32);
      const savedDepth = metadata.readUInt32LE(32);
      return new MerkleTree(db, name, savedDepth, rootHash);
    } catch (error) {
      const newTree = new MerkleTree(db, name, depth);
      await newTree.writeMetaData();
      return newTree;
    }
  }

  private async writeMetaData(batch?: LevelUpChain<string, Buffer>) {
    const data = Buffer.alloc(40);
    this.root.copy(data);
    data.writeUInt32LE(this.depth, 32);
    if (batch) {
      batch.put(this.name, data);
    } else {
      await this.db.put(this.name, data);
    }
  }

  getRoot() {
    return this.root;
  }

  async getHashPath(index: number) {
    let currentNodes = (await this.db.get(this.root)) as Buffer;
    const hashPath = new HashPath();
    console.log('currentNodes', currentNodes);

    for (let i = this.depth - 1; i >= 0; i--) {
      const [leftNode, rightNode] = [currentNodes.slice(0, 32), currentNodes.slice(32, 64)];
      hashPath.data[i] = [leftNode, rightNode];

      if (i !== 0) {
        const nextNode = this.isRight(index, this.depth - 1 - i) ? rightNode : leftNode;
        currentNodes = await this.db.get(nextNode);
      }
    }

    return hashPath;
  }

  async updateElement(index: number, value: Buffer) {
    const batch = this.db.batch();

    const updateRecursively = async (parent: Buffer, currentDepth: number): Promise<Buffer> => {
      if (currentDepth === this.depth) {
        return this.hasher.hash(value);
      }

      const nodes = (await this.db.get(parent)) as Buffer;
      let [leftNode, rightNode] = [nodes.slice(0, 32), nodes.slice(32, 64)];
      const isRight = this.isRight(index, currentDepth);
      const updatedNode = await updateRecursively(isRight ? rightNode : leftNode, currentDepth + 1);

      if (isRight) {
        rightNode = updatedNode;
      } else {
        leftNode = updatedNode;
      }

      const newParent = this.hasher.compress(leftNode, rightNode);
      batch.put(newParent, this.hasher.concat([leftNode, rightNode]));

      return newParent;
    };

    this.root = await updateRecursively(this.root, 0);
    await batch.write();
    await this.writeMetaData();

    return this.root;
  }

  isRight(index: number, currentDepth: number): number {
    const bitPosition = this.depth - currentDepth - 1;
    const mask = Math.pow(2, bitPosition);
    return index % (2 * mask) >= mask ? 1 : 0;
  }
}
