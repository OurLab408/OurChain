#!/bin/bash

space=" "
node=$1
cmd=$2

args=""

if [ "$#" -ge 3 ]
then
    shift 2
    for i
    do
        if [[ "$i" =~ "$space" ]]
        then 
            args="$args ${i@Q}"
        else
            args="$args $i"
        fi
    done
fi

if [ "$cmd" = "start" ]
then
    command="../src/bitcoind --datadir=node/$node"
elif [ "$cmd" = "kill" ]
then
    command="rm -rf node/$node/regtest"
else
    command="../src/bitcoin-cli --datadir=node/$node $cmd$args"
fi
echo -e "\033[42m run \033[0m$command"
echo "$command" | bash
