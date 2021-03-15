#!/bin/bash

pkill -9 -f trader.sh
pkill -9 -f miner.sh
pkill -9 -f bitcoind
rm -rf tmp/
rm -rf traderdir/
rm -rf minerdir/
mkdir -m 777 tmp
