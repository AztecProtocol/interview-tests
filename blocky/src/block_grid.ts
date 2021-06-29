import { Block } from './block';

export class BlockGrid {
  public grid: Block[][] = [];

  constructor(public numCols: number, public numRows: number) {
    for (let x = 0; x < numCols; x++) {
      const col = [];
      for (let y = 0; y < numRows; y++) {
        col.push(new Block());
      }
      this.grid.push(col);
    }
  }

  clicked(x: number, y: number) {
    console.log(`(${x}, ${y}): Implement me...`);
  }
}
