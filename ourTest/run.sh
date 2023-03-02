#!/bin/bash

space=" "
node=$1
cmd=$2
args=$3
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

if [ "$cmd" = "start" ]
then
    bitcoind --datadir=node/"$node"
elif [ "$cmd" = "test" ]
then
    echo "bitcoin-cli --datadir=node/$node $cmd $args"
elif [ "$cmd" = "kill" ]
then
    rm -rf node/"$node"/regtest
else
    bitcoin-cli --datadir=node/"$node" "$cmd" "$args"
fi