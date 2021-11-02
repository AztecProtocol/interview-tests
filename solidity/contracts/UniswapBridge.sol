// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2020 Spilsbury Holdings Ltd
pragma solidity >=0.6.6 <0.8.0;
pragma experimental ABIEncoderV2;

import {SafeMath} from '@openzeppelin/contracts/math/SafeMath.sol';
import {IERC20} from '@openzeppelin/contracts/token/ERC20/IERC20.sol';

import {UniswapV2Library} from '@uniswap/v2-periphery/contracts/libraries/UniswapV2Library.sol';
import {IUniswapV2Router02} from '@uniswap/v2-periphery/contracts/interfaces/IUniswapV2Router02.sol';

import {IDefiBridge} from './interfaces/IDefiBridge.sol';
import {Types} from './Types.sol';

// import 'hardhat/console.sol';

contract UniswapBridge is IDefiBridge {
    using SafeMath for uint256;

    address public immutable defiBridgeProxy;
    address public weth;

    IUniswapV2Router02 router;

    constructor(address _defiBridgeProxy, address _router) public {
        defiBridgeProxy = _defiBridgeProxy;
        router = IUniswapV2Router02(_router);
        weth = router.WETH();
    }

    receive() external payable {}

    function convert(
        Types.AztecAsset[4] calldata assets,
        uint64,
        uint256,
        uint256 inputValue
    )
        external
        payable
        override
        returns (
            uint256 outputValueA,
            uint256,
            bool isAsync
        )
    {
        require(msg.sender == defiBridgeProxy, 'UniswapBridge: INVALID_CALLER');
        isAsync = false;
        uint256[] memory amounts;
        uint256 deadline = block.timestamp;
        // TODO This should check the pair exists on UNISWAP instead of blindly trying to swap.

        if (assets[0].assetType == Types.AztecAssetType.ETH && assets[2].assetType == Types.AztecAssetType.ERC20) {
            address[] memory path = new address[](2);
            path[0] = weth;
            path[1] = assets[2].erc20Address;
            amounts = router.swapExactETHForTokens{value: inputValue}(0, path, defiBridgeProxy, deadline);
            outputValueA = amounts[1];
        } else if (
            assets[0].assetType == Types.AztecAssetType.ERC20 && assets[2].assetType == Types.AztecAssetType.ETH
        ) {
            address[] memory path = new address[](2);
            path[0] = assets[0].erc20Address;
            path[1] = weth;
            require(
                IERC20(assets[0].erc20Address).approve(address(router), inputValue),
                'UniswapBridge: APPROVE_FAILED'
            );
            amounts = router.swapExactTokensForETH(inputValue, 0, path, defiBridgeProxy, deadline);
            outputValueA = amounts[1];
        } else {
            // TODO what about swapping tokens?
            revert('UniswapBridge: INCOMPATIBLE_ASSET_PAIR');
        }
    }

    function canFinalise(
        Types.AztecAsset[4] calldata,
        uint64,
        uint256
    ) external view override returns (bool) {
        return false;
    }

    function finalise(
        Types.AztecAsset[4] calldata,
        uint64,
        uint256
    ) external payable override returns (uint256, uint256) {
        require(false);
    }
}
