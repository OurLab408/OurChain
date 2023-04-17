#!/bin/bash

space=" "
log=true
exc=true
node=$1

if [ "$node" = "--log" ]; then
    exc=false
    shift
    node=$1
elif [ "$node" = "--silent" ]; then
    log=false
    shift
    node=$1
fi

cmd=$2

args=""

if [ "$#" -ge 3 ]; then
    shift 2
    for i; do
        if [[ "$i" =~ "$space" ]]; then 
            args="$args ${i@Q}"
        else
            args="$args $i"
        fi
    done
fi


if [ "$cmd" = "mine" ]; then
    while [ 1 ]; do
        blocks=$(src/bitcoin-cli --datadir=ourTest/node/$node generate 1)
        sleep 2
    done
elif [ "$cmd" = "start" ]; then
    command="src/bitcoind --datadir=ourTest/node/$node"
elif [ "$cmd" = "kill" ]; then
    command="rm -rf ourTest/node/$node/regtest"
else
    command="src/bitcoin-cli --datadir=ourTest/node/$node $cmd$args"
fi
if [ "$log" = true ]; then
    echo -e "\033[42m run \033[0m$command"
fi
if [ "$exc" = true ]; then
    echo "$command" | bash
fi
