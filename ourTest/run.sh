#!/bin/bash

space=" "
node=$1
cmd=$2

if [ "$cmd" = "test" ]
then
    cmd2=$3
    shift 1
fi

args=$3
if [ "$#" -le 2 ]
then
    args=""
fi

if [ "$#" -ge 3 ]
then
    shift 3
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
    bitcoind --datadir=node/"$node"
elif [ "$cmd" = "test" ]
then
    echo "bitcoin-cli --datadir=node/$node $cmd2 $args"
elif [ "$cmd" = "kill" ]
then
    rm -rf node/"$node"/regtest
else
    echo "bitcoin-cli --datadir=node/$node $cmd $args" | bash
fi