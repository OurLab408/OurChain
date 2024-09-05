#!/bin/bash
# Leon Lin for Ourchain

mining() {
    ./src/bitcoin-cli getnewaddress > address.txt
    ./src/bitcoin-cli generatetoaddress $1 $(cat address.txt)
    sleep 5
    rm address.txt
}

deploycontract() {
    ./src/bitcoin-cli deploycontract ~/Desktop/ourchain/sample.cpp > log.txt
    # 使用 grep 和 awk 從 log.txt 文件中提取合約地址
    contract_address=$(grep "contract address" log.txt | awk -F'"' '{print $4}')
    rm log.txt
    echo "$contract_address"
}
# compile and install OurChain
make -j8 && make install && ldconfig
# start OurChain with regtest mode
./src/bitcoind --regtest --daemon -txindex
sleep 5
# print the balance of the mining address
./src/bitcoin-cli getbalance
mining 11
# deploy sample contract
contract_address=$(deploycontract)
echo "contract: $contract_address"
# try sample smart contract
mining 1
./src/bitcoin-cli callcontract "$contract_address" ""
mining 2
./src/bitcoin-cli dumpcontractmessage "$contract_address" ""
# general contract interface
./src/bitcoin-cli dumpcontractmessage "$contract_address" "get"
# stop OurChain
./src/bitcoin-cli stop