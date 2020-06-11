# XGM wallet protocol API

Wallet API has the same structure as Node Stratum API protocol (JSON RPC 2.0 over TCP connection) and should have an online connection to the node. However, it can work over HTTP if you will use --api_use_http=1 option, send POST requests to http://x.x.x.x:port/api/wallet in this case.

## How to use

Find wallet-api binary in the wallet subdirectory (or download from releases), place it near your wallet.db file and run with the following arguments:
```
Wallet API general options:
  -h [ --help ]                     list of all options
  -p [ --port ] arg (=10000)        port to start server on
  -n [ --node_addr ] arg            address of node
  --wallet_path arg (=wallet.db)    path to wallet file
  --pass arg                        password for the wallet
  --use_http arg (=0)               use JSON RPC over HTTP
  --ip_whitelist arg                IP whitelist

User authorization options:
  --use_acl arg (=0)                use Access Control List (ACL)
  --acl_path arg (=wallet_api.acl)  path to ACL file

TLS protocol options:
  --use_tls arg (=0)                use TLS protocol
  --tls_cert arg (=wallet_api.crt)  path to TLS certificate
  --tls_key arg (=wallet_api.key)   path to TLS private key
```
___________________________________________________________
```
./wallet-api --node_addr=node:por --pass=123 for example.
```
## Demo code

Here is a code on NodeJS to get all the UTXOs, for example:
```
var net = require('net');

var client = new net.Socket();
client.connect(10000, '127.0.0.1', function() {
	console.log('Connected');
	client.write(JSON.stringify(
		{
			jsonrpc: '2.0',
			id: 123,
			method: 'get_utxo',
			params: {}
		}) + '\n');
});

var acc = '';

client.on('data', function(data) {
	acc += data;

	// searching for \n symbol to find end of response
	if(data.indexOf('\n') != -1)
	{
		var res = JSON.parse(acc);

		console.log('Received:', res);

		client.destroy(); // kill client after server's response
	}
});

client.on('close', function() {
	console.log('Connection closed');
});
```

## Curl POST example

If you use JSONRPC over HTTP you can do POST requests using CURL.

You will need to specify 'use_http=1' in wallet-api.cfg to enable HTTP mode

Here is an example to get current wallet status.
```
curl -d '{"jsonrpc":"2.0","id":1,"method":"wallet_status"}' -H "Content-Type: application/json" -X POST http://x.x.x.x:port/api/wallet
```

# API

API will include the following methods:

    create_address
    validate_address
    addr_list
    delete_address
    edit_address
    tx_send
    tx_status
    tx_split
    tx_list
    tx_cancel
    wallet_status
    get_utxo

## create_address

Creates new receiver address.
```
-->

{
	"jsonrpc":"2.0",
	"id": 1,
	"method":"create_address",
	"params":
	{
		"expiration": "24h"
	}
}
```
where expiration can be expired/never/24h or omitted (24h is default value)
```
<--

{
	"jsonrpc":"2.0",
	"id": 1,
	"result":"472e17b0419055ffee3b3813b98ae671579b0ac0dcd6f1a23b11a75ab148cc67"
}
```
## validate_address

Just base check, validates if the address isn't garbage and belongs our elliptic curve. Also returns is_mine == true if address is found in the wallet DB.
```
-->

{
	"jsonrpc":"2.0",
	"id": 1,
	"method":"validate_address",
	"params":
	{
		"address" : "472e17b0419055ffee3b3813b98ae671579b0ac0dcd6f1a23b11a75ab148cc67"
	}
}
```

```
<--

{
	"jsonrpc":"2.0",
	"id": 1,
	"result" :
	{
		"is_valid" : true,
		"is_mine" : false,
	}
}
```

## addr_list

Get all the addresses in the wallet. Use own=true to get all the own addresses
```
-->

{
	"jsonrpc":"2.0",
	"id": 8,
	"method":"addr_list",
	"params":
	{
		"own" : true
	}
}
```

```
<--

{
	"id": 8,
	"jsonrpc": "2.0",
	"result":
	[
		{
			"address":
			"29510b33fac0cb20695fd3b836d835451e600c4224d8fb335dc1a68271deb9b6b5b",
			"category": "",
			"create_time": 1553174321,
			"duration": 1520,
			"expired": true,
			"label": "",
			"own": true
		}
	]
}
```
## delete_address

Delete specific address from the wallet.
```
-->

{
	"jsonrpc":"2.0",
	"id": 8,
	"method":"delete_address",
	"params":
	{
		"address" : "29510b33fac0cb20695fd3b836d835451e600c4224d8fb335dc1a68271deb9b6b5b"
	}
}
```

```
<--

{
	"id": 8,
	"jsonrpc": "2.0",
	"result": "done"
}
```
## edit_address

Edit specific address in the wallet, you can change the label or expiration to expired/never/24h or do both things at the same time.
```
-->

{
	"jsonrpc":"2.0",
	"id": 8,
	"method":"edit_address",
	"params":
	{
		"address" : "29510b33fac0cb20695fd3b836d835451e600c4224d8fb335dc1a68271deb9b6b5b",
		"comment" : "John Smith",
		"expiration" : "expired"
	}
}
```

```
<--

{
	"id": 8,
	"jsonrpc": "2.0",
	"result": "done"
}
```

## tx_send

Sends transactions with specific value to a given address.
```
-->

{
	"jsonrpc":"2.0",
	"id": 2,
	"method":"tx_send",
	"params":
	{
		"value" : 12342342,
		"fee" : 2,
		"from" : "472e17b0419055ffee3b3813b98ae671579b0ac0dcd6f1a23b11a75ab148cc67",
		"address" : "472e17b0419055ffee3b3813b98ae671579b0ac0dcd6f1a23b11a75ab148cc67",
		"comment" : "thank you!"
	}
}
```

value and fee are in Centum (1 XGM = 100000000 Centums)

from address can be ommited, new own address will be generated and registered in this case.

fee and comment can be omitted.

You can also preselect specific UTXOs for a transaction by adding coins array parameter, like
```
"coins" : "00057e8eca5673476e6f726d000000000000015d3ef79800", "00057e8eca1233476e6f726d000000000000015d3ef79800"
```

```
<--

{
	"jsonrpc":"2.0",
	"id": 2,
	"result":
	{
		"txId" : "10c4b760c842433cb58339a0fafef3db"
	}
}
```
Returns transaction id or error code.

## tx_status

Checks status of existing transaction. Status can be Pending(0), InProgress(1), Cancelled(2), Completed(3), Failed(4), Registering(5). More info here TX statuses description.
```
-->

{
	"jsonrpc":"2.0",
	"id": 4,
	"method":"tx_status",
	"params":
	{
		"txId" : "10c4b760c842433cb58339a0fafef3db"
	}
}
```

```
<--

{
	"jsonrpc":"2.0",
	"id": 4,
	"result":
	{
		"txId" : "10c4b760c842433cb58339a0fafef3db",
		"comment": "",
		"fee": 0,
		"kernel": "0000000000000000000000000000000000000000000000000000000000000000",
		"receiver": "472e17b0419055ffee3b3813b98ae671579b0ac0dcd6f1a23b11a75ab148cc67",
		"sender": "f287176bdd517e9c277778e4c012bf6a3e687dd614fc552a1ed22a3fee7d94f2",
		"status": 4,
                "status_string" : "Failed",
                "failure_reason" : "No inputs",
		"value": 12342342,
		"height" : 1055,
		"confirmations" : 3,
		"create_time" : 1551100217,
		"income" : false
	}
}
```
height and confirmations will be absent if transaction isn't in chain.

## tx_split

Creates a specific set of outputs with given set of values and session.

The session parameter is important in the following case:

Let's say you need to do a payout to a 1000 people, each with different amount. First you split the UTXOs you have using the 'split' method but then if you want to make another transaction you want to make sure these UTXOs are not used for any other payout. To do that you 'lock' them with the session id during split, and they will only be spent of the specific session id is provided in the 'send' method.
```
-->

{
	"jsonrpc":"2.0",
	"id": 5,
	"method":"tx_split",
	"params":
	{
		"coins" : [11, 12, 13, 50000000],
		"fee" : 3
	}
}
```

```
<--

{
	"jsonrpc":"2.0",
	"id": 5,
	"result":
	{
		"txId" : "10c4b760c842433cb58339a0fafef3db"
	}
}
```
coins and fee are in Centum

## wallet_status

Get current wallet status height/hash/available/...
```
-->

{
	"jsonrpc":"2.0",
	"id": 6,
	"method":"wallet_status"
}
```

```
<--

{
	"jsonrpc":"2.0",
	"id": 6,
	"result":
	{
	    "current_height" : 1055,
	    "current_state_hash" : "f287176bdd517e9c277778e4c012bf6a3e687dd614fc552a1ed22a3fee7d94f2",
	    "prev_state_hash" : "bd39333a66a8b7cb3804b5978d42312c841dbfa03a1c31fc2f0627eeed6e43f2",
	    "available": 100500,
	    "receiving": 123,
	    "sending": 0,
	    "maturing": 50,
	    "locked": 30,
	    "difficulty": 2.93914,
	}
}
```
available - a sum of available UTXOs you can spend
sending/receiving - a sum of UTXOs currently sending/receiving
maturing - a sum of UTXOs currently maturing
locked - not currently using, ignore it
difficulty - the latest blockchain PoW difficulty
get_utxo

## Get list of all unlocked UTXOs.

To do a pagination use count/skip parameters:

count - number of UTXO to get, all the UTXO will be returned by default.

skip - number of UTXO to skip, default is 0.
```
-->

{
	"jsonrpc":"2.0",
	"id": 6,
	"method":"get_utxo",
	"params" :
	{
		"count" : 10,
		"skip" : 0
	}
}
```

```
<--

{
	"jsonrpc":"2.0",
	"id": 6,
	"result":
	[
		{
			"id" : 123,
			"amount" : 12345,
			"maturity" : 60,
			"type" : "mine",
			"createTxId" : "10c4b760c842433cb58339a0fafef3db",
			"spentTxId" : "",
			"status" : 2,
			"status_string" : "maturing"
		}
	]
}
```
type can be fees, mine, norm, chng, ...

status can be Unavailable (0), Available (1), Maturing (2), Outgoing (3), Incoming (4), Spent (6)
tx_list

## Get all the transactions with specified status/height....

To do a pagination use count/skip parameters:

count - number of transactions to get, all the transactions will be returned by default.

skip - number of transactions to skip, default is 0.
```
-->

{
	"jsonrpc":"2.0",
	"id": 8,
	"method":"tx_list",
	"params":
	{
		"filter" :
		{
			"status":4,
			"height":1055,
		},
		"skip" : 0,
		"count" : 10
	}
}
```

```
<--

{
	"jsonrpc":"2.0",
	"id": 8,
	"result":
	[
		{
			"txId" : "10c4b760c842433cb58339a0fafef3db",
			"comment": "",
			"fee": 0,
			"kernel": "0000000000000000000000000000000000000000000000000000000000000000",
			"receiver": "472e17b0419055ffee3b3813b98ae671579b0ac0dcd6f1a23b11a75ab148cc67",
			"sender": "f287176bdd517e9c277778e4c012bf6a3e687dd614fc552a1ed22a3fee7d94f2",
			"status": 4,
			"status_string" : "Failed",
			"failure_reason" : "No inputs",
			"value": 12342342,
			"height" : 1055,
			"confirmations" : 0,
			"create_time" : 1551100217,
			"income" : false
		},
		{
			"txId" : "10c4b760c842433cb58339a0fafef3db",
			"comment": "",
			"fee": 0,
			"kernel": "0000000000000000000000000000000000000000000000000000000000000000",
			"receiver": "472e17b0419055ffee3b3813b98ae671579b0ac0dcd6f1a23b11a75ab148cc67",
			"sender": "f287176bdd517e9c277778e4c012bf6a3e687dd614fc552a1ed22a3fee7d94f2",
			"status": 4,
			"status_string" : "Failed",
			"failure_reason" : "No inputs",
			"value": 12342342,
			"height" : 1055,
			"confirmations" : 2,
			"create_time" : 1551100217,
			"income" : false
		}
	]
}
```
TX statuses description

Pending (0)     - initial state, a transaction is created, but not sent nowhere
InProgress (1)  - "Waiting for Sender/Waiting for Receiver" - to indicate that sender or receiver should come online to initiate the transaction
Canceled (2)    - "Cancelled" (by Sender, due to Rollback)
Completed (3)   - a transaction is completed
Failed (4)      - failed for some reason
Registering (5) - a transaction is taken care by the blockchain, some miner needs to PoW and to add it to a block, the block should be added to the blockchain

status_string is a string representation of status parameter.

height and confirmations will be absent if a transaction isn't in chain.

## tx_cancel

Cancels running transaction, return true if successfully canceled or error with the reason.
```
-->

{
	"jsonrpc":"2.0",
	"id": 4,
	"method":"tx_cancel",
	"params":
	{
		"txId" : "a13525181c0d45b0a4c5c1a697c8a7b8"
	}
}
```

```
<--

{
	"jsonrpc":"2.0",
	"id": 4,
	"result": true
}
```
## IP whitelistening

Add --ip_whitelist=192.168.0.1,192.168.0.2 parameter to enable access to the API by IP address.
User authorization

The API methods can have access rights if you enable Access Control List (ACL) in the command line using --use_acl=1 parameter)

create_address      - write access
validate_address    - read access
addr_list           - read access
delete_address      - write access
edit_address        - write access
tx_send             - write access
tx_status           - read access
tx_split            - write access
tx_cancel           - write access
get_utxo            - read access
tx_list             - read access
wallet_status       - read access

ACL file should look like a list with the access keys and read/write rights:

472e17b0419055ffee3b3813b98ae671579b0ac0dcd6f1a23b11a75ab148cc67 : write
bd39333a66a8b7cb3804b5978d42312c841dbfa03a1c31fc2f0627eeed6e43f2 : read
f287176bdd517e9c277778e4c012bf6a3e687dd614fc552a1ed22a3fee7d94f2 : read

Also, don't forget to send a user key in every JSONRPC request to the API now:
```
{"jsonrpc":"2.0","id":1,"method":"wallet_status","key":"bd39333a66a8b7cb3804b5978d42312c841dbfa03a1c31fc2f0627eeed6e43f2"}
```
TLS encryption

Add --use_tls=1 to enable TLS encryption. Also, you have to buy the certificates or create self signed certificates on your local machine and pass them using --tls_cert and --tls_key parameters.
