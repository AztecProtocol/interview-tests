// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2020 Spilsbury Holdings Ltd
pragma solidity >=0.6.6 <0.8.0;
pragma experimental ABIEncoderV2;

import {Types} from '../Types.sol';

interface IDefiBridge {
    /**
     * Input cases:
     * Case 1: 1 real asset.
     * Case 2: 1 virtual asset.
     * Case 3: 1 real asset 1 virtual asset.
     *
     * Output cases:
     * Case 1: 1 real asset.
     * Case 2: 2 real assets.
     * Case 3: 1 real asset 1 virtual asset.
     * Case 4: 1 virtual asset.
     *
     * Example use cases:
     * 1-1: Swapping.
     * 1-2: Swapping with incentives (2nd output reward token).
     * 1-3: Borrowing. Lock up collateral, get back loan asset and virtual position asset.
     * 1-4: Opening lending position OR Purchasing NFT. Input real asset, get back virtual asset representing NFT or position.
     * 2-1: Selling NFT. Input the virtual asset, get back a real asset.
     * 2-2: Closing a lending position. Get back original asset and reward asset.
     * 2-3: Claiming fees from an open position.
     * 2-4: Voting on a 1-4 case.
     * 3-1: Repaying a borrow. Return loan plus interest. Get collateral back.
     * 3-2: Repaying a borrow. Return loan plus interest. Get collateral plus reward token. (AAVE)
     * 3-3: Partial loan repayment.
     * 3-4: DAO voting stuff.
     *
     * This function is called from the DefiBridgeProxy after the tokens or ETH have been sent to the bridge.
     * This function should call the defi protocol, and return the output assets to the DefiBridgeProxy if they are ETH or tokens, unless convert returns isAsync = true.
     * @param assets assets.
     * @param inputValue the total amount input, if there are two input assets equal amounts of both will have been input.
     * @param interactionNonce a globally unique identifier for this defi interaction.
     * @param auxData passthrough data for the bridge contract (could contain data for e.g. slippage, nftID, etc.)
     * @return outputValueA the amount of outputAssetA returned from this interaction, should be 0 if async.
     * @return outputValueB the amount of outputAssetB returned from this interaction, should be 0 if async or bridge only returns 1 asset.
     * @return isAsync a flag to signal if this bridge interaction will return assets at a later date with a call finalise().
     */
    function convert(
        Types.AztecAsset[4] calldata assets,
        uint64 auxData,
        uint256 interactionNonce,
        uint256 inputValue
    )
        external
        payable
        returns (
            uint256 outputValueA,
            uint256 outputValueB,
            bool
        );

    /**
     * @dev This function is called via the DefiBridgeProxy.
     * @param interactionNonce the interaction nonce.
     * @return true if the asynchronous interaction denoted by interactionNonce can be finalised.
     */
    function canFinalise(
        Types.AztecAsset[4] calldata assets,
        uint64 auxData,
        uint256 interactionNonce
    ) external view returns (bool);

    /**
     * This function is called via the DefiBridgeProxy. It should transfer the output assets specified by original call to `convert` with `interactionNonce`.
     * The defi bridge proxy will check it has received the return values if the asset types are ETH or ERC20.
     * @param interactionNonce the interaction nonce of an async defi interaction being finalised.
     * @return outputValueA the return value of output asset A
     * @return outputValueB optional return value of output asset B
     */
    function finalise(
        Types.AztecAsset[4] calldata assets,
        uint64 auxData,
        uint256 interactionNonce
    ) external payable returns (uint256 outputValueA, uint256 outputValueB);
}
