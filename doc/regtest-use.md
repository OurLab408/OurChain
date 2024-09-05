# How to Bitcoin regtest

## Setup

1. Download [bitcoin core](https://bitcoin.org/en/download)
2. Unpack it wherever you want.
3. Create a directory named `data` inside the unpacked folder.
4. Create a directory named `.bitcoin` inside your home folder: `mkdir ~/.bitcoin`
5. Copy [bitcoin.conf](https://github.com/bitcoin/bitcoin/blob/master/share/examples/bitcoin.conf) into `~/.bitcoin`
6. Copy [rpcauth](https://github.com/bitcoin/bitcoin/blob/master/share/rpcauth/rpcauth.py) into `<unpacked_folder>/share/rpcauth`
7. Run `<unpacked_folder>/share/rpcauth/rpcauth.py <YOUR_USERNAME>`. **Keep the output somewhere**, you'll need it later.
8. Edit your `bitcoin.conf`.
9. Uncomment `regtest=0` and enable it: `regtest=1`
10. Uncomment `rpcuser=alice` and replace the value with your username.
11. Uncomment `rpcpassword=...` and replace the value with the password you got from `rpcauth`.
12. Uncomment `rpcauth=bob:...` and replace the value with the line you got from `rpcauth`.
13. _(Optional)_ Append `datadir=<unpacked_folder>/data` to the end of the file, where `<unpacked_folder>` is the absolute path to your unpacked folder.
14. Run `<unpacked_folder>/bin/bitcoind`
15. Open another terminal, and run `<unpacked_folder>/bin/bitcoin-cli getbalance`. It should return `0`.

Your first node is set. Now let's setup another one to connect to. We need to use another port and another data dir.

Run: `bitcoind -port=18445 -rpcport=8333 -datadir=<unpacked_folder>/data2`

## Generate a new address

We'll start by generating a new wallet address. We will use it for all our future operations, so **keep it somewhere**.

```
bitcoin-cli getnewaddress
```

## Generate some blocks

```
bitcoin-cli generatetoaddress 50 <youraddress>

# Check the blocks are mined
bitcoin-cli getblockcount # Should return 50
```

## Connect to other nodes

Let's connect to our second node: `bitcoin-cli addnode "127.0.0.1:18445" add`.  
To keep the connection after a restart, add the following to your `bitcoin.conf`: `addnode=127.0.0.1:18445`

Check if your nodes are connected: `bitcoin-cli getaddednodeinfo`.

Now check if the second node is synchronized: `bitcoin-cli -rpcport=8333 getblockcount`.

Let's mine some blocks and check if everything is synchronized:

```bash
bitcoin-cli generatetoaddress 25 "<your_address>"
bitcoin-cli getblockcount # Should return 75
bitcoin-cli -rpcport=8333 getblockcount # Should return 75
```

## Transfer funds

We got some funds from mining the first blocks. We will transfer them to our wallet on the second node.

```bash
address=`bitcoin-cli -rpcport=8333 getnewaddress`
bitcoin-cli sendtoaddress "$1" 10

# Check the funds have been received
bitcoin-cli -rpcport=8333 getwalletinfo
```

## Double spend

As we only have two nodes in the network, it's easy to make a 51% attack. We're going to double spend some bitcoin.

```bash
# Get a unspent output with a non-zero output
bitcoin-cli listunspent

# Create a raw transaction and sign it
# Don't forget to set the amount of your vout a little bit lower than the amount of your utxo so your tx has fees
transaction=`bitcoin-cli createrawtransaction '[{"txid":"<TX_ID>","vout":0}]' '{"$address":12.49}'`
bitcoin-cli signrawtransactionwithwallet "$transaction"
```

Keep the generated hex of the signed transaction. We need to disconnect both nodes and broadcast the transaction from the other node.

```bash
# Disconnect nodes
bitcoin-cli disconnectnode "127.0.0.1:18445"
bitcoin-cli -rpcport=8333 disconnectnode "127.0.0.1:18444"

# Broadcast from other node
bitcoin-cli -rpcport=8333 sendrawtransaction "<HEX>"
```

To make a double spend, we need to create another transaction with the same utxo and broadcast it on the chain where the first one wasn't broadcasted.

```bash
# Generate a second address to make it easier to differenciate both transactions
address=`bitcoin-cli -rpcport=8333 getnewaddress`

transaction=`bitcoin-cli createrawtransaction '[{"txid":"<TX_ID>","vout":0}]' '{"$address":12.49}'`
bitcoin-cli signrawtransactionwithwallet "$transaction"
bitcoin-cli sendrawtransaction "<HEX>"
```

We need to mine some blocks so when we join the two nodes, the longest chain is kept. (In our case, the one with the second transaction).

```bash
bitcoin-cli generate 50

# Join
bitcoin-cli addnode "127.0.0.1:18445" add
```

Check that the first transaction doesn't exist anymore:

```bash
bitcoin-cli -rpcport=8333 listtransactions
```

The last one should have the address we used for the second transaction. The double-spend worked!
