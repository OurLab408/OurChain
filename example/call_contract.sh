file1=server.c
file2=client.c

sudo kill -9 $(ps aux | grep bitcoind | sed -n '1p' | awk '{print $2;}') 2>/dev/null
sudo kill -9 $(ps aux | grep bitcoind | sed -n '2p' | awk '{print $2;}') 2>/dev/null
rm ~/.bitcoin/regtest -rf
rm ~/.bitcoin/blocks -rf
killall ourcontract-rt
killall ourcontract-rt
bitcoind -regtest -txindex -reindex -daemon

sleep 3
bitcoin-cli generate 101
bitcoin-cli deploycontract ${file1}
bitcoin-cli generate 1
#bitcoin-cli deploycontract ${file2} 
#bitcoin-cli generate 1