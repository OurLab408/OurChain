cd ~/OurChain/ourTest/zk
zokrates compile -i ex1.zok --stdlib-path $ZOKRATES_STDLIB
zokrates setup

cd ~/OurChain/ourTest
./run.sh A kill
./run.sh B kill
./run.sh A start
sleep 2
TMP=$(./run.sh A generate 101)
CID=$(./run.sh A deployzkcontract zk/ex1.c zk/ex1.zok zk/proving.key zk/verification.key)
[[ $CID =~ address.+([0-9a-f]{64}) ]]
CID=${BASH_REMATCH[1]}
echo "contract address: $CID"
TMP=$(./run.sh A generate 1)

./run.sh B start
sleep 2
PROOF=$(./run.sh B generatezkproof $CID "2147483647 274876858367" | grep -P '[\da-f]{64}' -o | sed 'N;s/\n//' | tr '\n' ' ')
TMP=$(./run.sh B generate 101)

echo "valid proof ($PROOF):"
./run.sh B callcontract $CID foo bob $PROOF
TMP=$(./run.sh B generate 1)
./run.sh B dumpcontractmessage $CID

echo "invalid proof (${PROOF/0/1}):"
./run.sh B callcontract $CID foo bob ${PROOF/0/1}
TMP=$(./run.sh B generate 1)
./run.sh B dumpcontractmessage $CID

./run.sh A stop
./run.sh B stop

