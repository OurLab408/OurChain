#!/bin/bash

i=1
while [ $i -le $3 ]
do
    bitcoin-cli -regtest -rpcpassword=123 -rpcport=$1 generate 1
    ((i++))
    sleep $2
done
