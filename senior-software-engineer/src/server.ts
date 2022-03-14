import { Block } from './block_source';
import { MemoryFifo } from './fifo';
import { HashPath } from './merkle_tree';
import { BlockSource } from './block_source';
import { HashPathSource, TreeState } from './hash_path_source';
import { WorldStateDb } from './world_state_db';
import { ServerState, ServerStateDb } from './server_state';

export class Server implements HashPathSource {
  private serverState: ServerState = { lastBlock: -1 };

  public constructor(
    private worldStateDb: WorldStateDb,
    private serverStateDb: ServerStateDb,
    private blockSource: BlockSource,
  ) {}

  /**
   * Fully synchronises chain state before returning.
   * Starts processing of any new subsequent new blocks.
   */
  public async start() {
    this.serverState = await this.serverStateDb.readState();

    console.log(`Synchronising chain state from ${this.serverState.lastBlock + 1}...`);

    // Implement.

    this.printState();
  }

  /**
   * Stops processing new blocks / requests, returns once everythings complete.
   */
  public async stop() {}

  getTreeState(): Promise<TreeState> {
    // Implement.
    throw new Error('Method not implemented.');
  }

  getHashPath(index: number): Promise<HashPath> {
    // Implement.
    throw new Error('Method not implemented.');
  }

  private printState() {
    console.log(`Data size: ${this.worldStateDb.getSize()}`);
    console.log(`Data root: ${this.worldStateDb.getRoot().toString('hex')}`);
  }
}
