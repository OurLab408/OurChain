#!/bin/bash

mining() {
    ./src/bitcoin-cli getnewaddress > address.txt
    ./src/bitcoin-cli generatetoaddress "$1" "$(cat address.txt)"
    sleep 5
    rm address.txt
}

deploycontract() {
    local contract_file="$1"
    if [ ! -f "$contract_file" ]; then
        echo "Error: Contract file '$contract_file' not found."
        return 1
    fi
    ./src/bitcoin-cli deploycontract "$contract_file" > log.txt
    contract_address=$(grep "contract address" log.txt | awk -F'"' '{print $4}')
    rm log.txt
    echo "$contract_address"
}

# compile and install OurChain
make -j$(nproc) && sudo make install
# start OurChain with regtest mode
./src/bitcoind --regtest --daemon -txindex
sleep 5
# print the balance of the mining address
./src/bitcoin-cli getbalance
mining 11
# deploy contract
contract_address=$(deploycontract ./src/test/test_contract.cpp)
echo "contract: $contract_address"
# call smart contract
mining 1
./src/bitcoin-cli callcontract "$contract_address" "increment" "1"
./src/bitcoin-cli callcontract "$contract_address" "increment" "3"
./src/bitcoin-cli callcontract "$contract_address" "increment" "5"
./src/bitcoin-cli callcontract "$contract_address" "increment" "7"
./src/bitcoin-cli callcontract "$contract_address" "increment" "9"
./src/bitcoin-cli callcontract "$contract_address" "decrement" "30"
./src/bitcoin-cli callcontract "$contract_address" "increment" "11"
./src/bitcoin-cli callcontract "$contract_address" "increment" "13"
./src/bitcoin-cli callcontract "$contract_address" "increment" "15"
./src/bitcoin-cli callcontract "$contract_address" "increment" "17"
./src/bitcoin-cli callcontract "$contract_address" "increment" "19"
mining 2
# stop OurChain
./src/bitcoin-cli --regtest stop
