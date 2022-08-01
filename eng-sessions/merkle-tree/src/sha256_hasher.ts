import { createHash } from 'crypto';

/**
 * Implements a Hasher using the sha256 algorithm.
 */
export class Sha256Hasher {
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
