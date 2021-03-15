#!/bin/bash

sudo pkill -9 -f trader.sh
sudo pkill -9 -f miner.sh
sudo pkill -9 -f bitcoind
rm -rf tmp/
rm -rf traderdir/
rm -rf minerdir/
mkdir -m 777 tmp
