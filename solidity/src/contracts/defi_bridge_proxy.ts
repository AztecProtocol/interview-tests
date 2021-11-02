import { Provider } from '@ethersproject/providers';
import { Contract, ContractFactory, Signer } from 'ethers';
import abi from '../artifacts/contracts/DefiBridgeProxy.sol/DefiBridgeProxy.json';

export interface SendTxOptions {
  gasPrice?: bigint;
  gasLimit?: number;
}

export enum AztecAssetType {
  NOT_USED,
  ETH,
  ERC20,
  VIRTUAL,
}

export interface AztecAsset {
  id?: number;
  assetType?: AztecAssetType;
  erc20Address?: string;
}

const assetToArray = (asset: AztecAsset) => [
  asset.id || 0,
  asset.erc20Address || '0x0000000000000000000000000000000000000000',
  asset.assetType || 0,
];

export class DefiBridgeProxy {
  private contract: Contract;

  constructor(public address: string, provider: Provider) {
    this.contract = new Contract(this.address, abi.abi, provider);
  }

  static async deploy(signer: Signer) {
    const factory = new ContractFactory(abi.abi, abi.bytecode, signer);
    const contract = await factory.deploy();
    return new DefiBridgeProxy(contract.address, signer.provider!);
  }

  async deployBridge(signer: Signer, abi: any, args: any[]) {
    const factory = new ContractFactory(abi.abi, abi.bytecode, signer);
    const contract = await factory.deploy(this.contract.address, ...args);
    return contract.address;
  }

  /**
   * @param signer Signer sending the tx.
   * @param bridgeAddress Target bridge contract address.
   * @param assets [inputAssetA, inputAssetB, outputAssetA, outputAssetB].
   * @param auxInputData 8 bytes of opaque data sent to the bridge contract.
   * @param interactionNonce The current unique interaction nonce.
   * @param inputValue To total input value.
   * @param options Ethereum tx send options.
   * @returns
   */
  async convert(
    signer: Signer,
    bridgeAddress: string,
    assets: AztecAsset[],
    auxInputData: bigint,
    interactionNonce: bigint,
    inputValue: bigint,
    options: SendTxOptions = {},
  ) {
    const contract = new Contract(this.contract.address, this.contract.interface, signer);
    const { gasLimit, gasPrice } = options;

    const tx = await contract.convert(
      bridgeAddress,
      assets.map(assetToArray),
      auxInputData,
      interactionNonce,
      inputValue,
      { gasLimit, gasPrice },
    );
    const receipt = await tx.wait();

    const parsedLogs = receipt.logs
      .filter((l: any) => l.address == contract.address)
      .map((l: any) => contract.interface.parseLog(l));

    const { outputValueA, outputValueB, isAsync } = parsedLogs[0].args;

    return {
      isAsync,
      outputValueA: BigInt(outputValueA.toString()),
      outputValueB: BigInt(outputValueB.toString()),
    };
  }

  async canFinalise(bridgeAddress: string, assets: AztecAsset[], auxInputData: bigint, interactionNonce: bigint) {
    return await this.contract.canFinalise(bridgeAddress, assets.map(assetToArray), auxInputData, interactionNonce);
  }

  async finalise(
    signer: Signer,
    bridgeAddress: string,
    assets: AztecAsset[],
    auxInputData: bigint,
    interactionNonce: bigint,
    options: SendTxOptions = {},
  ) {
    const contract = new Contract(this.contract.address, this.contract.interface, signer);
    const { gasLimit, gasPrice } = options;
    const tx = await contract.finalise(bridgeAddress, assets.map(assetToArray), auxInputData, interactionNonce, {
      gasLimit,
      gasPrice,
    });
    const receipt = await tx.wait();

    const parsedLogs = receipt.logs
      .filter((l: any) => l.address == contract.address)
      .map((l: any) => contract.interface.parseLog(l));

    const { outputValueA, outputValueB, isAsync } = parsedLogs[0].args;

    return {
      isAsync,
      outputValueA: BigInt(outputValueA.toString()),
      outputValueB: BigInt(outputValueB.toString()),
    };
  }
}
