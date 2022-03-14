import { createHash } from 'crypto';
import { Hasher } from './hasher';

/**
 * Implements a Hasher using the sha256 algorithm.
 */
export class Sha256Hasher implements Hasher {
  compress(lhs: Buffer, rhs: Buffer): Buffer {
    return createHash('sha256')
      .update(Buffer.concat([lhs, rhs]))
      .digest();
  }

  hash(data: Buffer): Buffer {
    return createHash('sha256').update(data).digest();
  }
}
