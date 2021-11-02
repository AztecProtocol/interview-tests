# Aztec Technical Challenge

**WARNING: Do not fork this repository or make a public repository containing your solution. Either copy it to a private repository or submit your solution via other means.**

Links to solutions may be sent to charlie@aztecprotocol.com.

## The task

This project contains an example bridge and unit test that will do a synchronous swap of assets on Uniswap V2. There are 3 options you can choose of what's considered to be increasing difficulty.

### Option 1

Improve the existing Uniswap bridge to:

- Support token to token swaps.
- Use aux data to specify a maximum price slippage.
- Optionally improve gas usage.

### Option 2

Write a new DeFi bridge contract that interacts with your preferred L1 protocol. It maybe desirable to leverage features such as virtual assets and asychronous completion. Example ideas would be to support exchanges with liquidity pools and limit orders, or lending and borrowing protocols and liquidity mining.

### Option 3

Make the given Uniswap bridge contract net away trade balances when it has both sides of a market e.g `ETH/DAI` & `DAI/ETH`, only trading the delta via Uniswap.

- Assume two different bridge ids pointing to the same bridge address.
- One call to convert would take input ETH output DAI.
- Second call to convert would take input DAI output ETH.
- You can assume a fixed order to the two calls.
- Use the asychronous flow.

## What is a bridge?

A bridge is a layer 1 solidity contract that conforms a DeFi protocol to the interface the Aztec rollup expects. This allows the Aztec rollup contract to interact with the DeFi protocol via the bridge.

A bridge contract models any layer 1 DeFi protocol as a synchronous or asynchronous asset swap. You can specify up to two input assets and two output assets per bridge.

## How does this work?

Users who have shielded funds on Aztec can construct a zero-knowledge proof instructing the Aztec rollup contract to make an external L1 contract call.

Aztec Connect works by batching L2 transactions by bridge id together in a rollup, and batch executing them against L1 bridge contracts. The results are later disseminated to the users privately by their relevant input ratios.

## Batching bridge interactions.

Rollup providers are incentivised to batch any transaction with the same bridge id. This reduces the cost of the L1 transaction for similar trades. A bridge id is structured as:

```
BridgeId (248 bits)
(auxData || bitConfig || outputAssetB || outputAssetA || inputAssetB || inputAssetA || bridgeAddressId)
    64         32            30              30                30            30               32

BitConfig (32 bits)
(unused || firstAssetVirtual || secondAssetValid || secondAssetVirtual)
   29             1                     1                   1
```

For this test, the bridge id is assumed to have already been parsed out into a more manageable form.

### Virtual Assets

Aztec uses the concept of virtual assets or "position" tokens to represent a share of assets held by a bridge contract. This is far more gas efficient than minting ERC20 tokens. These are used when the bridge holds an asset that Aztec doesn't support, i.e. Uniswap Position NFT's or other non-fungible assets.

If the output asset of any interaction is specified as virtual, the user will receive encrypted notes on Aztec representing their share of the position, but no tokens or ETH need to be transferred. The position tokens have an `assetId` that is the `interactionNonce` of the DeFi Bridge call. This is globably unique. Virtual assets can be used to construct complex flows, such as entering or exiting LP positions. i.e. One bridge contract can have multiple flows which are triggered using different input assets.

### Auxillary Data

This is 64 bits of bridge specific data that can be passed through Aztec to the bridge contract. It is defined by the bridge contract and is opaque to Aztec.

## Bridge Contract Interface

The bridge contract interface can be seen [here](./contracts/interfaces/IDefiBridge.sol), and has further information on the responsiblities of each function.

## Anatomy of a rollup transaction with defi bridge interactions.

It may help to have some context around how the rollup processor contract processes each defi bridge.

For each bridge id:

1. Parse the bridge id into constituent parts.
2. Transfer any tokens or ETH to the bridge address.
3. Call `convert` for each bridge id as described in the interface.
4. If `isAsync == false` check the bridge sent the DefiBridgeProxy the output assets if ETH or ERC20.

Finally:

1. For any calls to `convert` that return `isAsync == true`, check if the interaction can be finalised by calling `canFinalise`.
2. For any calls to `canFinalise` that return `true`, finalise the interaction by calling `finalise`.
3. Check the bridge sent the DefiBridgeProxy the output assets if ETH or ERC20.
