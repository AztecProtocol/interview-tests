# Aztec Blocky Test

**WARNING: Do not fork this repository or make a public repository containing your solution. Either copy it to a private repository or submit your solution via other means.**

Links to solutions may be sent to charlie@aztecprotocol.com.

## To get started

```sh
yarn install
yarn start
```

Navigate to `http://localhost:8080`.

`yarn test` or `yarn test --watch` to run the unit tests on the terminal.

## Step 1

Implement `clicked` to remove all blocks of the same colour that are connected to the target element, then allow the blocks above the removed to "fall down" (similar to Tetris but you should click a block to have connected blocks removed).

E.g. given:

![Initial state](https://trottski.s3.amazonaws.com/snaps/initial.jpg)

After clicking one of the bottom right blue boxes it should then look like this:

![state 2](https://trottski.s3.amazonaws.com/snaps/stage2.jpg)

## Step 2

Improve the game as you see fit. Take as long as you like, be as creative as you like. Demonstrate what you can do.
