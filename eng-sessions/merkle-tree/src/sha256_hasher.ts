import { createHash } from 'crypto';

/**
 * Implements a Hasher using the sha256 algorithm.
 */
export class Sha256Hasher {
  private emptyHashCache: Buffer[] = [];
  constructor() {
    this.emptyHashCache.push(this.hash(Buffer.alloc(64, 0)));
  }

  emptyHash(depth: number): Buffer {
    if (depth < 0) {
      throw new Error('Depth cannot be negative');
    }
    while (depth >= this.emptyHashCache.length) {
      const lastHash = this.emptyHashCache[this.emptyHashCache.length - 1];
      const newHash = this.compress(lastHash, lastHash);
      this.emptyHashCache.push(newHash);
    }
    return this.emptyHashCache[depth];
  }

  /**
   * Given two roots, the left hand subtree root, and the right hand subtree root, return a digest representing
   * the new tree root.
   */
  compress(lhs: Buffer, rhs: Buffer): Buffer {
    return createHash('sha256')
      .update(Buffer.concat([lhs, rhs]))
      .digest();
  }

  /**
   * Given `data` which is to be become an entry in the tree, return a digest that represents that data.
   */
  hash(data: Buffer): Buffer {
    return createHash('sha256').update(data).digest();
  }
}
