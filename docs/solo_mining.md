# XGM Solo Mining Guide
- Adapted from Raskul's guide
- With help from Freshminer and Mikeyb

## Installation
- From [Releases](https://github.com/freenetcoder/defis/releases/tag/100.1) download the following
  - defis-wallet-cli
  - defis-node
- Extract downloads into a folder called `defis-mining`

## Setup
- Create SSL certificate & key in `defis-mining` directory
  - `openssl req -newkey rsa:4096 -nodes -keyout stratum.key -x509 -days 365 -out stratum.crt
- Create a file called `stratum.api.keys` and add a alphanumeric string
  - Example: `Zmohq90bfgMvLB4gjL9NS5PeAJ7CReXO3G7aZ0aW`
  - You can add 1 string per line.  These are used at the `user` in your mining client
- Edit `defis-wallet.cfg` file removing the `#` before `pass` and `node_addr`
  - `pass=abcdefghijklmnopqrstuvwxyz1234567890`
  - `node_addr=127.0.0.1:10005`
  - Leave the rest as is
- Initialize wallet by running this command in a terminal
  - `./defis-wallet init`
  - Save the passphrase and address before continuing
- Export miner key by running this command in a terminal
  - `./defis-wallet export_miner_key --subkey=1`
  - Take the key and put in the `defis-node.cfg` file on the `key_mine=` line
- Export owner key by running this command in a terminal
  - `./defis-wallet export_owner_key`
  - Take the key and put in the `defis-node.cfg` file on the `key_owner=` line
- Get the `pass` in `defis-wallet.cfg` and add it to the `pass=` line in `defis-node.cfg`
- Add these entries to `defis-node.cfg`
  - `port=10005`
  - `log_level=verbose`
  - `file_log_level=verbose`
  - `mining_threads=0`
  - `peer=64.225.32.192:10150,167.172.32.97:10150,157.245.71.80:10150,134.209.89.56:10005`
  - `stratum_port=3333`
    - Make sure your firewall is allowing connections to this port if mining externally
  - `stratum_secrets_path=.`
- Start the node with the following command
  - `./defis-node`
  - Leave this terminal open to keep the node running
- Wait for node to fully synchronize
- Open another terminal window in `defis-mining` directory and run the command
  - `./defis-wallet listen`
  - Leave this terminal open to keep the wallet listening
- Point your miner at the ip address and port of this device with key from stratum.api.keys instead wallet address
 
## Config Examples
- `defis-wallet.cfg`
```
pass=abcdefghijklmnopqrstuvwxyz1234567890
node_addr=127.0.0.1:10005
```

- `defis-node.cfg`
```
port=10005
log_level=verbose
file_log_level=verbose
mining_threads=0
peer=64.225.32.192:10150,167.172.32.97:10150,157.245.71.80:10150,134.209.89.56:10005
stratum_port=3333
stratum_secrets_path=.
key_mine=export_miner_key string
key_owner=export_owner_key string
pass=abcdefghijklmnopqrstuvwxyz1234567890
```
