cd ~/OurChain/ourTest/zk
zokrates compile -i ex1.zok --stdlib-path $ZOKRATES_STDLIB
zokrates setup

cd ~/OurChain
./test.sh A kill
./test.sh B kill
./test.sh A start
sleep 2
TMP=$(./test.sh A generate 101)
CID=$(./test.sh A deployzkcontract ourTest/zk/ex1.c ourTest/zk/ex1.zok ourTest/zk/proving.key ourTest/zk/verification.key)
[[ $CID =~ address.+([0-9a-f]{64}) ]]
CID=${BASH_REMATCH[1]}
echo "contract address: $CID"
TMP=$(./test.sh A generate 1)

./test.sh B start
sleep 2
PROOF=$(./test.sh B generatezkproof $CID "2147483647 274876858367" | grep -P '0x[\da-f]{64}' -o | sed 'N;s/\n//' | sed 's/0x//g' | tr '\n' ' ')
TMP=$(./test.sh B generate 101)

echo "valid proof ($PROOF):"
./test.sh B callcontract $CID foo bob $PROOF
TMP=$(./test.sh B generate 1)
./test.sh B dumpcontractmessage $CID

echo "invalid proof (${PROOF/0/1}):"
./test.sh B callcontract $CID foo bob ${PROOF/0/1}
TMP=$(./test.sh B generate 1)
./test.sh B dumpcontractmessage $CID

./test.sh A stop
./test.sh B stop

