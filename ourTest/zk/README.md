# ZK in OurChain demo

\* 以下所有步驟可以 `./fulltest.sh` 代替

### 1. 簡易撰寫ZK程式碼
  已準備在 [ex1/zk](/ourTest/zk/ex1/zk) 內
### 2. 編譯與製作proving key & verivication key
```bash
cd ~/OurChain/ourTest/zk
zokrates compile -i ex1.zok
# 若失敗則使用以下一行
zokrates compile -i ex1.zok --stdlib-path $ZOKRATES_STDLIB
zokrates setup
```
### 3. 簡易撰寫智能合約
  已準備在 [ex1/code.c](/ourTest/zk/ex1/code.c) 內
### 4. deploy
```bash
cd ~/OurChain
./test.sh A start
./test.sh A generate 101
./test.sh A deployzkcontract ourTest/zk/ex1/code.c ourTest/zk/ex1/zk ourTest/zk/ex1/proving.key ourTest/zk/ex1/verification.key
./test.sh A generate 1
```
### 5. 開另一個 node 測試
```bash
./test.sh B start
# 生成證明 範例中的程式可用"2147483647 274876858367"通過
./test.sh B proofzk <contract address> "2147483647 274876858367"
# 先挖礦才有錢 call smart contract
./test.sh B generate 101
# 將 proof 當作 function 的參數使用
./test.sh B callcontract <contract address> foo <隨便一個名字> <字串1> <字串2> <字串3> <字串4>
# 上鏈
./test.sh B generate 1
# 查看結果 ➡️ Proved by <名字>
./test.sh B dumpcontractmessage <contract address> 
# 嘗試 call 錯誤的 proof
./test.sh B callcontract <contract address> foo <隨便一個名字> <字串1> <字串1> <字串1> <字串1>
# 上鏈
./test.sh B generate 1
# 查看結果 ➡️ <空值>
./test.sh B dumpcontractmessage <contract address> 
```
### 6. 善後
```bash
./test.sh A stop
./test.sh B stop
./test.sh A kill
./test.sh B kill
```
