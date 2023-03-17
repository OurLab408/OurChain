# dev by docker

## 環境設置

參考 https://www.docker.com/ 安裝 docker

參考 https://www.docker.com/products/docker-desktop/ 安裝 docker desktop

參考 https://code.visualstudio.com/ 安裝 vscode

參考 https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers 安裝 vscode container 插件

## 第一次執行

在本專案根目錄執行

```bash
# 安裝 image
docker build .
# 獲取 id
docker image ls
# 從 image 生成 container 且啟動
docker run -it [IMAGE ID]
```

設置環境變數與啟動測試

```bash
# 貼上設置內容
vim /root/.bitcoin/bitcoin.conf
# 啟動 (BUG:會遇到吃不到代碼更新, 所以直接執行 local)
./src/bitcoind --regtest --daemon
# 停止
./src/bitcoin-cli stop
# 教學
./src/bitcoin-cli help
# 獲取餘額
./src/bitcoin-cli getbalance
# 挖礦給自己
./src/bitcoin-cli generate 1
# 發布合約
./src/bitcoin-cli deploycontract ~/Desktop/ourchain/sample.c
# 執行合約 (can check info in ~/.bitcoin/regtest/contracts)
./src/bitcoin-cli callcontract "contract address when deploy" "arg1" "arg2" ...
# 獲取合約列印訊息
./src/bitcoin-cli dumpcontractmessage "contract address"
```

設置內容

```
server=1
rpcuser=test
rpcpassword=test
rpcport=8332
rpcallowip=0.0.0.0/0
regtest=1
```

啟動後可以在 Desktop 找到專案

## 重複使用 container

```bash
# 獲取所有 container ID
docker container ls -a
# 啟動之前的 container
docker start [CONTAINER ID]
```

## 檔案編輯與執行

### 編輯

- 打開 VS Code
- 最左邊一排 icon 內，點 Remote Explorer
- 按下正確的 Container 後，出現的圖標中按下 Attach to Container
- 接著會自動開一個新的 VS Code，左下角綠底的部分會顯示該 Container 的 Image ID
- 最左邊一排 icon 內，點 Explorer
- 路徑選擇 ourchain 所在的位置打開即可

### 執行

- VS Code 最上方工具列選擇 Terminal > New Terminal 或是用快捷鍵 Ctrl+Shift+`
- 也可以利用 docker desktop 開啟 container 內部 Terminal

## 刪除

請利用 docker desktop 照直覺操作即可

## 操作筆記

編譯 libourcontract (一般 make 改不到 dll 的相依)

```
make && make install && ldconfig
```

kill bitcoin

```
pidof bitcoind
kill -9 pid
```

kill contract 殭屍線程

```
netstat -lpn |grep 18444
kill pid
```
