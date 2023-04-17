#!/bin/bash

cd ~/OurChain/ourTest/zk/ex1
zokrates compile -i zk --stdlib-path $ZOKRATES_STDLIB
zokrates setup

cd ~/OurChain
./test.sh A kill
./test.sh B kill
./test.sh A start
sleep 2
./test.sh --log A generate 101
TMP=$(./test.sh A generate 101)
./test.sh --log A deployzkcontract ourTest/zk/ex1/code.c ourTest/zk/ex1/zk ourTest/zk/ex1/proving.key ourTest/zk/ex1/verification.key
CID=$(./test.sh --silent A deployzkcontract ourTest/zk/ex1/code.c ourTest/zk/ex1/zk ourTest/zk/ex1/proving.key ourTest/zk/ex1/verification.key)
[[ $CID =~ address.+([0-9a-f]{64}) ]]
CID=${BASH_REMATCH[1]}
echo "contract address: $CID"
./test.sh --log A generate 1
TMP=$(./test.sh A generate 1)

./test.sh B start
sleep 2
./test.sh --log B proofzk $CID "2147483647 274876858367"
PROOF=$(./test.sh --silent B proofzk $CID "2147483647 274876858367")
./test.sh --log B generate 101
TMP=$(./test.sh B generate 101)

echo "valid proof:"
./test.sh B callcontract $CID foo bob $PROOF
./test.sh --log B generate 1
TMP=$(./test.sh B generate 1)
./test.sh B dumpcontractmessage $CID

echo "invalid proof:"
./test.sh B callcontract $CID foo bob ${PROOF/0/1}
./test.sh --log B generate 1
TMP=$(./test.sh B generate 1)
./test.sh B dumpcontractmessage $CID

./test.sh A stop
./test.sh B stop

