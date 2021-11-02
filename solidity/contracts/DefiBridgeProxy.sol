// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2020 Spilsbury Holdings Ltd
pragma solidity >=0.6.10 <0.8.0;
pragma experimental ABIEncoderV2;

import {SafeMath} from '@openzeppelin/contracts/math/SafeMath.sol';
import {IDefiBridge} from './interfaces/IDefiBridge.sol';
import {Types} from './Types.sol';

// import 'hardhat/console.sol';

contract DefiBridgeProxy {
    using SafeMath for uint256;

    bytes4 private constant BALANCE_OF_SELECTOR = 0x70a08231; // bytes4(keccak256('balanceOf(address)'));
    bytes4 private constant TRANSFER_SELECTOR = 0xa9059cbb; // bytes4(keccak256('transfer(address,uint256)'));
    bytes4 private constant DEPOSIT_SELECTOR = 0xb6b55f25; // bytes4(keccak256('deposit(uint256)'));
    bytes4 private constant WITHDRAW_SELECTOR = 0x2e1a7d4d; // bytes4(keccak256('withdraw(uint256)'));

    event AztecBridgeInteraction(
        address indexed bridgeAddress,
        uint256 outputValueA,
        uint256 outputValueB,
        bool isAsync
    );

    receive() external payable {}

    function getBalance(address assetAddress) internal view returns (uint256 result) {
        assembly {
            if iszero(assetAddress) {
                // This is ETH.
                result := balance(address())
            }
            if assetAddress {
                // Is this a token.
                let ptr := mload(0x40)
                mstore(ptr, BALANCE_OF_SELECTOR)
                mstore(add(ptr, 0x4), address())
                if iszero(staticcall(gas(), assetAddress, ptr, 0x24, ptr, 0x20)) {
                    // Call failed.
                    revert(0x00, 0x00)
                }
                result := mload(ptr)
            }
        }
    }

    function transferTokens(
        address assetAddress,
        address to,
        uint256 amount
    ) internal {
        assembly {
            let ptr := mload(0x40)
            mstore(ptr, TRANSFER_SELECTOR)
            mstore(add(ptr, 0x4), to)
            mstore(add(ptr, 0x24), amount)
            // is this correct or should we forward the correct amount
            if iszero(call(gas(), assetAddress, 0, ptr, 0x44, ptr, 0)) {
                // Call failed.
                revert(0x00, 0x00)
            }
        }
    }

    function convert(
        address bridgeAddress,
        Types.AztecAsset[4] calldata assets,
        uint256 auxData,
        uint256 interactionNonce,
        uint256 inputValue
    )
        external
        returns (
            uint256 outputValueA,
            uint256 outputValueB,
            bool isAsync
        )
    {
        if (assets[0].assetType == Types.AztecAssetType.ERC20) {
            // Transfer totalInputValue to the bridge contract if erc20. ETH is sent on call to convert.
            transferTokens(assets[0].erc20Address, bridgeAddress, inputValue);
        }

        if (assets[1].assetType == Types.AztecAssetType.ERC20) {
            // Transfer totalInputValue to the bridge contract if erc20. ETH is sent on call to convert.
            transferTokens(assets[1].erc20Address, bridgeAddress, inputValue);
        }

        uint256 tempValueA;
        uint256 tempValueB;

        if (assets[2].assetType != Types.AztecAssetType.VIRTUAL) {
            tempValueA = getBalance(assets[2].erc20Address);
        }

        if (assets[3].assetType != Types.AztecAssetType.VIRTUAL) {
            tempValueB = getBalance(assets[3].erc20Address);
        }

        // Call bridge.convert(), which will return output values for the two output assets.
        // If input is ETH, send it along with call to convert.
        IDefiBridge bridgeContract = IDefiBridge(bridgeAddress);
        (outputValueA, outputValueB, isAsync) = bridgeContract.convert{
            value: assets[0].assetType == Types.AztecAssetType.ETH ? inputValue : 0
        }(
            assets,
            uint64(auxData),
            interactionNonce,
            inputValue
        );

        if (
            assets[2].assetType != Types.AztecAssetType.VIRTUAL &&
            assets[2].assetType != Types.AztecAssetType.NOT_USED
        ) {
            require(
                outputValueA == SafeMath.sub(getBalance(assets[2].erc20Address), tempValueA),
                'DefiBridgeProxy: INCORRECT_ASSET_VALUE'
            );
        }

        if (
            assets[3].assetType != Types.AztecAssetType.VIRTUAL &&
            assets[3].assetType != Types.AztecAssetType.NOT_USED
        ) {
            require(
                outputValueB == SafeMath.sub(getBalance(assets[3].erc20Address), tempValueB),
                'DefiBridgeProxy: INCORRECT_ASSET_VALUE'
            );
        }

        if (isAsync) {
            require(outputValueA == 0 && outputValueB == 0, 'DefiBridgeProxy: ASYNC_NONZERO_OUTPUT_VALUES');
        }

        emit AztecBridgeInteraction(bridgeAddress, outputValueA, outputValueB, isAsync);
    }

    function canFinalise(
        address bridgeAddress,
        Types.AztecAsset[4] calldata assets,
        uint64 auxData,
        uint256 interactionNonce
    ) external view returns (bool ready) {
        IDefiBridge bridgeContract = IDefiBridge(bridgeAddress);
        (ready) = bridgeContract.canFinalise(
            assets,
            auxData,
            interactionNonce
        );
    }

    function finalise(
        address bridgeAddress,
        Types.AztecAsset[4] calldata assets,
        uint64 auxData,
        uint256 interactionNonce
    ) external returns (uint256 outputValueA, uint256 outputValueB) {
        uint256 tempValueA;
        uint256 tempValueB;
        if (assets[2].assetType != Types.AztecAssetType.VIRTUAL) {
            tempValueA = getBalance(assets[2].erc20Address);
        }

        if (assets[3].assetType != Types.AztecAssetType.VIRTUAL) {
            tempValueB = getBalance(assets[3].erc20Address);
        }

        IDefiBridge bridgeContract = IDefiBridge(bridgeAddress);

        require(bridgeContract.canFinalise(
            assets,
            auxData,
            interactionNonce), 'DefiBridgeProxy: NOT_READY');

        (outputValueA, outputValueB) = bridgeContract.finalise(
            assets,
            auxData,
            interactionNonce
        );

        if (
            assets[2].assetType != Types.AztecAssetType.VIRTUAL &&
            assets[2].assetType != Types.AztecAssetType.NOT_USED
        ) {
            require(
                outputValueA == SafeMath.sub(getBalance(assets[2].erc20Address), tempValueA),
                'DefiBridgeProxy: INCORRECT_ASSET_VALUE'
            );
        }

        if (
            assets[3].assetType != Types.AztecAssetType.VIRTUAL &&
            assets[3].assetType != Types.AztecAssetType.NOT_USED
        ) {
            require(
                outputValueB == SafeMath.sub(getBalance(assets[3].erc20Address), tempValueB),
                'DefiBridgeProxy: INCORRECT_ASSET_VALUE'
            );
        }

        emit AztecBridgeInteraction(bridgeAddress, outputValueA, outputValueB, false);
    }
}
