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

## Time

It's expected you take no more than half a day. If you complete the algorithm sooner, feel free to be creative to improve the game further.

## Task

Implement `clicked` to remove all blocks of the same colour that are connected to the target element, then allow the blocks above the removed to "fall down".

E.g. given:

![Initial state](https://trottski.s3.amazonaws.com/snaps/initial.jpg)

After clicking one of the bottom right blue boxes it should then look like this:

![state 2](https://trottski.s3.amazonaws.com/snaps/stage2.jpg)
