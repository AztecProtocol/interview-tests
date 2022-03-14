import { Sha256Hasher } from './sha256_hasher';

export class HashPath {
  constructor(public data: Buffer[][] = []) {}

  public toBuffer() {
    return Buffer.concat(this.data.flat());
  }

  /**
   * Creates a new `HashPath` instance from a buffer previously created from `toBuffer`.
   */
  static fromBuffer(buf: Buffer) {
    return new HashPath(
      [...new Array(buf.length / 64)].map((_, i) => [
        buf.slice(i * 2 * 32, i * 2 * 32 + 32),
        buf.slice(i * 2 * 32 + 32, i * 2 * 32 + 64),
      ]),
    );
  }

  /**
   * Validates (returns true or false), that the merkle tree represented by `root`, contains `leafData` at `index`.
   */
  validate(root: Buffer, index: number, leafData: Buffer, hasher = new Sha256Hasher()) {
    // Implement
  }
}
