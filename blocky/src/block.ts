export enum Colour {
  RED,
  GREEN,
  BLUE,
  ORANGE,
}

export class Block {
  public colour: Colour;

  constructor() {
    this.colour = Colour[Colour[Math.floor(Math.random() * (Colour.ORANGE + 1))] as keyof typeof Colour];
  }
}
