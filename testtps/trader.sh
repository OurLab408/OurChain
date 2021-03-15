#!/bin/bash

while [ 0 -le 1 ] 
do
	i=1
	s=$((bitcoin-cli -regtest -rpcpassword=123 -rpcport=$1 sendtoaddress mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn 0.00001 >> tmp/tmp$1.txt) 2>&1)
	while [ $i -le 200 ]
	do
#		bitcoin-cli -regtest -rpcpassword=123 -rpcport=$1 getbalance >> tmp/tmp$1.txt
#		bitcoin-cli -regtest -rpcpassword=123 -rpcport=$1 getblockcount >> tmp/tmp$1.txt
#		bitcoin-cli -regtest -rpcpassword=123 -rpcport=$1 getblockhash `bitcoin-cli -regtest -rpcpassword=123 -rpcport=$1 getblockcount` >> tmp/tmp$1.txt
		e=$((bitcoin-cli -regtest -rpcpassword=123 -rpcport=$1 sendtoaddress mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn 0.00001 >> tmp/tmp$1.txt) 2>&1)
		if [ "$e" != "$s" ]
		then
			v1=2
			v2=`shuf -i 0-99 -n 1`
			v3="."
			sleep "$v1$v3$v2"
		fi
	done
done
echo "finish"
bitcoin-cli -regtest -rpcpassword=123 -rpcport=$1 stop
