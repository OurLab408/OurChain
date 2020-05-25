#!/bin/bash
#lsof -i -P -n | grep bitcoin

# Stop User Node
bitcoin-cli -regtest -datadir=/home/david/.bitcoin/ -conf=/home/david/.bitcoin/bitcoin.conf stop

bitcoin-cli -regtest -datadir=/home/david/.bitcoinB -conf=/home/david/.bitcoinB/bitcoin.conf stop

bitcoin-cli -regtest -datadir=/home/david/.bitcoinC -conf=/home/david/.bitcoinC/bitcoin.conf stop

bitcoin-cli -regtest -datadir=/home/david/.bitcoinD -conf=/home/david/.bitcoinD/bitcoin.conf stop

bitcoin-cli -regtest -datadir=/home/david/.bitcoinE -conf=/home/david/.bitcoinE/bitcoin.conf stop

# Stop Oracle Node
bitcoin-cli -regtest -datadir=/home/david/.bitcoinOracle/ -conf=/home/david/.bitcoinOracle/bitcoin.conf stop

# Clean data, remove generated blocks
echo Bitcoin regtest folders cleaned 
rm -rf ~/.bitcoin/regtest
rm -rf ~/.bitcoinB/regtest
rm -rf ~/.bitcoinC/regtest
rm -rf ~/.bitcoinD/regtest
rm -rf ~/.bitcoinE/regtest
rm -rf ~/.bitcoinOracle/regtest