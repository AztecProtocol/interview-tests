import { Colour } from './block';
import { BlockGrid } from './block_grid';

describe('BlockGrid', () => {
  it('should create blocks with one of the valid colours', () => {
    const blockGrid = new BlockGrid(10, 10);

    blockGrid.grid.forEach(col => {
      col.forEach(block => {
        expect(block).not.toBeNull();
        expect(block!.colour).toBeLessThanOrEqual(Colour.ORANGE);
      });
    });
  });

  it('should perform correct algorithm when clicked', () => {
    // Implement me.
  });
});
