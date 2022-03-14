import { LevelUp, LevelUpChain } from 'levelup';
import { Hasher } from './hasher';
import { HashPath } from './hash_path';

const MAX_DEPTH = 32;
const LEAF_BYTES = 64; // All leaf values are 64 bytes.

/**
 * The merkle tree, in summary, is a data structure with a number of indexable elements, and the property
 * that it is possible to provide a succinct proof (HashPath) that a given piece of data, exists at a certain index,
 * for a given merkle tree root.
 */
export class MerkleTree {
  private root!: Buffer;

  /**
   * Constructs a new MerkleTree instance, either initializing an empty tree, or restoring pre-existing state values.
   * Use the async static `new` function to construct.
   *
   * @param db Underlying leveldb.
   * @param hasher Hasher for hashing leaf value, and compressing left/right node hashes.
   * @param name Name of the tree, to be used when restoring/persisting state.
   * @param depth The depth of the tree, to be no greater than MAX_DEPTH.
   * @param size When restoring, you need to provider the size.
   * @param root When restoring, you need to provide the root.
   */
  constructor(
    private db: LevelUp,
    private hasher: Hasher,
    private name: string,
    private depth: number,
    private size: number = 0,
    root?: Buffer,
  ) {
    if (!(depth >= 1 && depth <= MAX_DEPTH)) {
      throw Error('Bad depth');
    }

    // Implement.
  }

  /**
   * Constructs or restores a new MerkleTree instance with the given `name` and `depth`.
   * The `db` contains the tree data.
   */
  static async new(db: LevelUp, leafHasher: Hasher, name: string, depth = MAX_DEPTH) {
    const meta: Buffer = await db.get(Buffer.from(name)).catch(() => {});
    if (meta) {
      const root = meta.slice(0, 32);
      const depth = meta.readUInt32LE(32);
      const size = meta.readUInt32LE(36);
      return new MerkleTree(db, leafHasher, name, depth, size, root);
    } else {
      const tree = new MerkleTree(db, leafHasher, name, depth);
      await tree.writeMeta();
      return tree;
    }
  }

  private async writeMeta(batch?: LevelUpChain<string, Buffer>) {
    const data = Buffer.alloc(40);
    this.root.copy(data);
    data.writeUInt32LE(this.depth, 32);
    data.writeUInt32LE(this.size, 36);
    if (batch) {
      batch.put(this.name, data);
    } else {
      await this.db.put(this.name, data);
    }
  }

  getRoot() {
    return this.root;
  }

  getSize() {
    return this.size;
  }

  async getElement(index: number) {
    // Implement.
    return Buffer.alloc(0);
  }

  /**
   * Returns the hash path for `index`.
   * e.g. To return the HashPath for index 2, return the nodes marked `*` at each layer.
   *     d0:                                            [ root ]
   *     d1:                      [*]                                               [*]
   *     d2:         [*]                      [*]                       [ ]                     [ ]
   *     d4:   [ ]         [ ]          [*]         [*]           [ ]         [ ]          [ ]        [ ]
   */
  async getHashPath(index: number) {
    // Implement.
    return new HashPath();
  }

  /**
   * Updates the tree with `value` at `index`. Returns the new tree root.
   */
  async updateElement(index: number, value: Buffer) {
    // Implement.
    return this.root;
  }
}
