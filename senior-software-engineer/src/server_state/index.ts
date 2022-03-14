import { pathExists, readJson, writeJson } from 'fs-extra';

export interface ServerState {
  lastBlock: number;
}

export interface ServerStateDb {
  readState(): Promise<ServerState>;
  writeState(state: ServerState): Promise<void>;
}

export class MemoryServerStateDb {
  private state: ServerState = { lastBlock: -1 };

  async readState(): Promise<ServerState> {
    return this.state;
  }

  async writeState(state: ServerState) {
    this.state = state;
  }
}

export class FileServerStateDb {
  constructor(private dbPath: string) {}

  async readState(): Promise<ServerState> {
    if (await pathExists(this.dbPath)) {
      return await readJson(this.dbPath);
    }
    return { lastBlock: -1 };
  }

  async writeState(state: ServerState) {
    await writeJson(this.dbPath, state);
  }
}
