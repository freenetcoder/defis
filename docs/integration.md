# Integration Defis guide for exchanges/pools/blockexplorers

## Getting binaries

Сhoose a branch "[mainnet](https://github.com/freenetcoder/defis/tree/mainnet)"- it's the latest released version, working with real money. You can get binaries from [Github Releases - Binaries for integration](https://github.com/freenetcoder/defis/releases/tag/100.1) or build yourself from the sources.

Detailed instructions on how to build a project for Windows, Linux, Mac platforms you can find [here](https://github.com/freenetcoder/defis/blob/mainnet/README.md).

    Add -DXGM_NO_QT_UI_WALLET=On command line parameter to the Cmake if you need only CLI version of the wallet without UI and QT5 library dependencies.

You will need to build defis-node, defis-wallet, wallet-api and explorer-node (or download with configs)

## Initializing Wallet

To create a new wallet run the following command (wallet.db will be created in the working folder):
```
$ ./defis-wallet init
```
Output example for init operation:

```
Generated seed phrase:

	large;taste;shock;swear;pencil;way;shine;sentence;merge;toddler;practice;liar;

	IMPORTANT

	Your seed phrase is the access key to all the cryptocurrencies in your wallet.
	Print or write down the phrase to keep it in a safe or in a locked vault.
	Without the phrase you will not be able to recover your money.
======

```
You can also restore wallet with your seed_phrase if you've already had it.
### To get an airdrop from Grimm need to restore your Defis (XGM) wallet using seed phrase from Grimm network. You will receive the same balance as Grimm on 444400 hard fork block height.
```
$ ./defis-wallet restore --seed_phrase="a; b; c; ... ;"
```
## Export Owner Key
Once wallet is initialised, you have to export your Owner Key (it will be needed in the future to start own node).
```
The purpose of the `owner key` is to allow all nodes mining for you to be aware of all mining
rewards mined by other nodes so that you would only need to connect to one node to collect all
the rewards into your wallet. While in most other cryptocurrencies this is done by simply mining
to a single address you control, in Mimblewimble it is not as simple since there are no addresses
and the mining rewards should be coded with unique blinding factors which are deterministically
derived from the `master key`, and then tagged by the single `owner key`.
```
Output example for export_owner_key operation:
```
$ ./defis-wallet export_owner_key

Owner Viewer key: kY9lb6eqWGlcUYG0eAR//fUyXtAuAv4fVfEFfWsTVmFNn++zI73KFpngwNJNigzzopZaeGXNs9m5729lPLtvFY1Blyti/uRsOmNqdS7HqpRimr3OFWZyATboQ5UvnOQey3OWLa/6mQZdOT6F
```
Owner Key should be kept secret. Owner Key does not allow to spend coins, however it will allow to see all coins mined for you by all miners that use this Owner Кey.

## Starting a Node

Run the node with your Owner Key and make sure it has completed the synchronization with the network:
```
$ ./defis-node --peer=ip:port --key_owner=kY9lb6eqWGlcUYG0eAR//fUyXtAuAv4fVfEFfWsTVmFNn++zI73KFpngwNJNigzzopZaeGXNs9m5729lPLtvFY1Blyti/uRsOmNqdS7HqpRimr3OFWZyATboQ5UvnOQey3OWLa/6mQZdOT6F

```
List of starting peers:
```
- 64.225.32.192:10150
- 167.172.32.97:10150
- 157.245.71.80:10150
- 134.209.89.56:10005
```
## Wallet API

With Wallet API you can

    check current wallet status and (balance)
    get all your UTXO/transactions list
    create/verify address
    send funds and cancel transactions
    make a split of UTXO - create a specific set of outputs with given set of values

There are two ways to send JSON RPC commands to the API,

    using TCP socket
    via HTTP calls.

With TCP you can hold only one connection, send JSON commands separated with \n symbol and receive callbacks from the API, in the one hand it's more flexible but a bit difficult to implement in the other hand. With HTTP you can do POST requests using curl command.

So, to start API with HTTP support use the command:
```
$ ./wallet-api --node_addr=x.x.x.x:port --api_use_http=1
where node_addr is your node address and port.
```
See detailed [wallet API documentation](https://github.com/freenetcoder/defis/blob/mainnet/docs/walletAPI.md)

## Node Explorer API

With this API you can get information about current blockchain state and blocks using simple GET requests.

To run explorer use the command:
```
$ ./explorer-node --peer ip:port --api_port=8080
```
[See detailed explorer API documentation](https://github.com/freenetcoder/defis/blob/mainnet/docs/explorerAPI)
