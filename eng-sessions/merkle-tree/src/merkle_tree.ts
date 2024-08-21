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
    
    // Implement. (DONE)
    if (root) {
        this.root = root;
      } else {
        this.root = this.hasher.emptyHash(depth);
      }
  }

  /**
   * Constructs or restores a new MerkleTree instance with the given `name` and `depth`.
   * The `db` contains the tree data.
   */
  static async new(db: LevelUp, name: string, depth = MAX_DEPTH) {
    const meta: Buffer = await db.get(Buffer.from(name)).catch(() => {});
    if (meta) {
      const root = meta.slice(0, 32);
      const depth = meta.readUInt32LE(32);
      return new MerkleTree(db, name, depth, root);
    } else {
      const tree = new MerkleTree(db, name, depth);
      await tree.writeMetaData();
      return tree;
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

  /**
   * Returns the hash path for `index`.
   * e.g. To return the HashPath for index 2, return the nodes marked `*` at each layer.
   *     d0:                                            [ root ]
   *     d1:                      [*]                                               [*]
   *     d2:         [*]                      [*]                       [ ]                     [ ]
   *     d3:   [ ]         [ ]          [*]         [*]           [ ]         [ ]          [ ]        [ ]
   */
  async getHashPath(index: number): Promise<HashPath> {
    // Implement. (DONE)
    const pathData: Buffer[][] = [];
    let depth = 0;

    while (depth < this.depth) {
      const isEven = index % 2 === 0;
      const siblingIndex = isEven ? index + 1 : index - 1;
      try {
        const [siblingHash, nodeHash] = await Promise.all([
            this.readNode(depth, siblingIndex),
            this.readNode(depth, index)
        ]);
        // Add the node hash and sibling hash to the path data
        pathData.push(isEven ? [nodeHash, siblingHash] : [siblingHash, nodeHash]);
      } catch (error) {
        console.error(`Error reading nodes at depth ${depth}, index ${index}:`, error);
        throw error;
      }
      // Move up to the next depth
      depth++;
      index >>= 1;
    }
    return new HashPath(pathData);
  }

  // Read node
  async readNode(depth: number, index: number): Promise<Buffer> {
    const key = this.generateKey(depth, index);
    try {
      const nodeHash: Buffer = await this.db.get(key.toString());
      return nodeHash;
    } catch (error) {
      return this.hasher.emptyHash(depth);
    }
  }

  // Generate unique key for DB
  private generateKey(depth: number, index: number): Buffer {
    const buffer = Buffer.alloc(8);
    buffer.writeUInt32LE(depth, 0);
    buffer.writeUInt32LE(index, 4);
    return this.hasher.hash(Buffer.concat([Buffer.from(this.name), buffer]));
  }

  // Write node
  private async writeNode(depth: number, index: number, value: Buffer, batch?: LevelUpChain<string, Buffer>) {
    if (value.length > LEAF_BYTES) {
      throw new Error(`Bad leaf value: value length ${value.length} exceeds maximum allowed ${LEAF_BYTES}`);
    }
    const key = this.generateKey(depth, index);
    if (batch) {
      batch.put(key.toString(), value);
    } else {
      await this.db.put(key.toString(), value);
    }
  }

  /**
   * Updates the tree with `value` at `index`. Returns the new tree root.
   */
  async updateElement(index: number, value: Buffer) {
    // Implement. (DONE)
    let depth = 1;
    let hashValue = this.hasher.hash(value);

    await this.writeNode(0, index, hashValue);

    while (depth <= this.depth) {
      const isEven = index % 2 === 0;
      const siblingIndex = index % 2 === 0 ? index + 1 : index - 1;
      try {
        // Read sibling hash
        const siblingHash = await this.readNode(depth - 1, siblingIndex);
        // Hash the updated value with the sibling hash to get the parent hash
        hashValue = isEven
            ? this.hasher.compress(hashValue, siblingHash)
            : this.hasher.compress(siblingHash, hashValue);
        // Write the new hash to the parent node
        await this.writeNode(depth, index >> 1, hashValue);
      } catch (error) {
          console.error(`Error updating element at depth ${depth}, index ${index}:`, error);
          throw error;
      }

      // Move up to the next depth
      depth++;
      index >>= 1;
    }
    // Update and return root
    this.root = hashValue;
    await this.writeMetaData();
    return this.root;
  }
}
