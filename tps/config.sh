#!/bin/bash
# Leon Lin for Ourchain

mining() {
    ./src/bitcoin-cli generate $1
}

deploycontract() {
    ./src/bitcoin-cli deploycontract ~/Desktop/ourchain/tps/tps.cpp > log.txt
    # 使用 grep 和 awk 从 log.txt 文件中提取合同地址
    contract_address=$(grep "contract address" log.txt | awk -F'"' '{print $4}')
    rm log.txt
    echo "$contract_address"
}

make -j8 && make install && ldconfig
./src/bitcoind --regtest --daemon -txindex
sleep 5
mining 30
contract_address=$(deploycontract)
echo "contract: $contract_address"
mining 1
for i in {1..300}
do
    ./src/bitcoin-cli callcontract "$contract_address" ""
done
# print current time, should have second precision
start=$(date +%s)
mining 2
end=$(date +%s)
./src/bitcoin-cli dumpcontractmessage "$contract_address" ""
# 一般化合約介面
./src/bitcoin-cli dumpcontractmessage "$contract_address" "get"

# print time difference
echo "Time elapsed: $((end-start)) seconds" >> ./tps/result.json
height=$(bitcoin-cli getblockcount)
bitcoin-cli getblock $(bitcoin-cli getblockhash $((height))) >> ./tps/result.json
bitcoin-cli getblock $(bitcoin-cli getblockhash $((height-1))) >> ./tps/result.json
bitcoin-cli getblock $(bitcoin-cli getblockhash $((height-2))) >> ./tps/result.json
bitcoin-cli getblock $(bitcoin-cli getblockhash $((height-3))) >> ./tps/result.json

./src/bitcoin-cli stop