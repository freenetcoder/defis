# Integration Defis guide for exchanges/pools

## Getting binaries

Сhoose a branch "[mainnet](https://github.com/freenetcoder/defis/tree/mainnet)"- it's the latest released version, working with real money. You can get binaries from [Github Releases - Binaries for integration]() or build yourself from the sources.

Detailed instructions on how to build a project for Windows, Linux, Mac platforms you can find [here](https://github.com/freenetcoder/defis/blob/mainnet/README.md).

    Add -DXGM_NO_QT_UI_WALLET=On command line parameter to the Cmake if you need only CLI version of the wallet without UI and QT5 library dependencies.

You will need to build defis-node and defis-wallet (or download with configs)

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
