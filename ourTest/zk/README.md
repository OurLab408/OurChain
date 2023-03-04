# ZK in OurChain

demo 教學

## 1. 環境設置
- 安裝 [Docker Desktop](https://www.docker.com/products/docker-desktop/)
- 安裝 [VSCode](https://code.visualstudio.com/)
- 安裝 [VSCode Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

## 2. 安裝 OurChain 與 ZoKrates (細節請見 [Dockerfile](/ourTest/zk/Dockerfile))
```bash
cd ourTest/zk
# 安裝 image 並命名為 ourchain
docker build -t ourchain .
# 從 ourchain 這個 image 生成 container，命名為 OurChain1，並啟動
docker run -it --name OurChain1 ourchain
# 第二次開始只需使用下方指令就能重複使用 OurChain1
docker start OurChain1
```
## 3. 簡易撰寫ZK程式碼
  已準備在 [ex1.zok](/ourTest/zk/ex1.zok) 內
## 4. 編譯與製作proving key & verivication key
```bash
cd ~/OurChain/ourTest/zk
zokrates compile -i ex1.zok
# 若失敗則使用以下一行
zokrates compile -i ex1.zok --stdlib-path $ZOKRATES_STDLIB
zokrates setup
```
## 5. 簡易撰寫智能合約
  已準備在 [ex1.c](/ourTest/zk/ex1.c) 內
## 6. deploy
```bash
cd ~/OurChain/ourTest
# 注意 ourTest/node/A/bitcoin.conf 應設定好
./run.sh A start
./run.sh A generate 101
./run.sh A deployzkcontract zk/ex1.c zk/ex1.zok zk/proving.key zk/verification.key
./run.sh A generate 1
```
## 7. 開另一個 node 測試
```bash
cd ~/OurChain/ourTest
# 注意 ourTest/node/B/bitcoin.conf 應設定好
./run.sh B start
# 生成證明 範例中的程式可用"2147483647 274876858367"通過
./run.sh B generatezkproof <contract address> "2147483647 274876858367"
# 確認證明無誤後 把證明轉成適當的樣子 (4個128長度的字串)
./run.sh B generatezkproof <contract address> "2147483647 274876858367" | grep -P '[\da-f]{64}' -o | sed 'N;s/\n//' | tr '\n' ' ' > proof
# 先挖礦才有錢 call smart contract
./run.sh B generate 101
# 將 proof 當作 function 的參數使用
./run.sh B callcontract <contract address> foo <隨便一個名字> <字串1> <字串2> <字串3> <字串4>
# 上鏈
./run.sh B generate 1
# 查看結果 ➡️ Proved by <名字>
./run.sh B dumpcontractmessage <contract address> 
# 嘗試 call 錯誤的 proof
./run.sh B callcontract <contract address> foo <隨便一個名字> <字串1> <字串1> <字串1> <字串1>
# 上鏈
./run.sh B generate 1
# 查看結果 ➡️ <空值>
./run.sh B dumpcontractmessage <contract address> 
```
## 8. 善後
```bash
./run.sh A stop
./run.sh B stop
./run.sh A kill
./run.sh B kill
```
