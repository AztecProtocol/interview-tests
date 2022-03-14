import { Server } from './server';
import { LocalBlockSource } from './block_source/local_block_source';
import levelup from 'levelup';
import memdown from 'memdown';
import { InternalWorldStateDb } from './world_state_db';
import { Block } from './block_source';
import { HashPath } from './merkle_tree';
import { MemoryServerStateDb } from './server_state';

describe('server test', () => {
  let server: Server;
  let worldStateDb: InternalWorldStateDb;
  let serverStateDb: MemoryServerStateDb;
  let blockSource: LocalBlockSource;

  const randomBlock = (id: number): Block => ({
    id,
    dataStartIndex: id * 2,
    created: new Date(),
    leafData: [Buffer.alloc(32, id * 2), Buffer.alloc(32, id * 2 + 1)],
  });

  const validateHashPath = (path: HashPath, root: Buffer, index: number) => {
    expect(path.validate(root, index, Buffer.alloc(32, index))).toBe(true);
  };

  beforeEach(async () => {
    const db = levelup(memdown());
    worldStateDb = new InternalWorldStateDb(db);
    serverStateDb = new MemoryServerStateDb();
    blockSource = new LocalBlockSource();
    server = new Server(worldStateDb, serverStateDb, blockSource);
    await worldStateDb.init();
  });

  afterEach(async () => {
    await server.stop();
  });

  it('should sync with new blocks', async () => {
    await server.start();

    expect((await server.getTreeState()).size).toBe(0);

    await blockSource.addBlock(randomBlock(0));
    await blockSource.addBlock(randomBlock(1));

    const { size, root } = await server.getTreeState();
    expect(size).toBe(4);
    expect(root.toString('hex')).toEqual('d75b9c13d280c162faa7fa330e8b424c67dcb6d4c6aa0345e3681016a3a2c446');
  });

  it('should get hash path', async () => {
    await server.start();

    expect((await server.getTreeState()).size).toBe(0);

    await blockSource.addBlock(randomBlock(0));
    await blockSource.addBlock(randomBlock(1));

    const { root } = await server.getTreeState();

    for (let i = 0; i < 4; i++) {
      validateHashPath(await server.getHashPath(i), root, i);
    }
  });

  it('should sync with pre-exising blocks', async () => {
    for (let i = 0; i < 10; ++i) {
      await blockSource.addBlock(randomBlock(i));
    }

    await server.start();

    const { size, root } = await server.getTreeState();
    expect(size).toBe(20);
    expect(root.toString('hex')).toEqual('21ed5b20d6fd82b9d7346c18aae79d075c3a5f1a96ca91235a247545f34679f2');
  });

  it('should sync from previous sync point', async () => {
    await server.start();

    await blockSource.addBlock(randomBlock(0));
    await blockSource.addBlock(randomBlock(1));

    await server.stop();

    server = new Server(worldStateDb, serverStateDb, blockSource);
    const spy = jest.spyOn(blockSource, 'start');
    await server.start();

    expect(spy).toHaveBeenCalledWith(2);
  });
});
