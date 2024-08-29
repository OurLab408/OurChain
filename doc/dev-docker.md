# dev by docker

## 環境設置

參考 https://www.docker.com/ 安裝 docker

參考 https://www.docker.com/products/docker-desktop/ 安裝 docker desktop

參考 https://code.visualstudio.com/ 安裝 vscode

參考 https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers 安裝 vscode container 插件

## 第一次執行

在本專案根目錄執行

```bash
# 安裝 image (don't want cache => `docker build --no-cache -t our-chain .`)
docker build -t our-chain .
# 從 image 生成 container 且啟動
docker run --name our-chain -it our-chain
# 開發外部應用(導出端口)
docker run --name our-chain -it -p 8332:8332 our-chain
```

設置環境變數與啟動測試

```bash
# 啟動 (BUG:會遇到吃不到代碼更新, 所以直接執行 local)
./src/bitcoind --regtest --daemon -txindex
# 停止
./src/bitcoin-cli stop
# 教學
./src/bitcoin-cli help
# 獲取餘額
./src/bitcoin-cli getbalance
# 挖礦給自己
./src/bitcoin-cli generate 1
# 發布合約
./src/bitcoin-cli deploycontract ~/Desktop/ourchain/sample.cpp
# 執行合約 (can check info in ~/.bitcoin/regtest/contracts)
./src/bitcoin-cli callcontract "contract address when deploy" "arg1" "arg2" ...
# 獲取合約狀態(此為 pure 操作)
./src/bitcoin-cli dumpcontractmessage "contract address" ""
```

can use `bash ./mytest.sh` run contract commands

相關位置

- 在 `/root/Desktop/ourchain` 可以找到專案
- 在 `/root/.bitcoin/bitcoin.conf` 找到設置
- 在 `/root/.bitcoin/regtest/contracts` 找到智能合約

## 重複使用 container

```bash
# 獲取所有 container ID
docker container ls -a
# 啟動之前的 container
docker start [CONTAINER ID]
```

## 發布 container

1. 在 dockerfile 中加入 `ENTRYPOINT ["bitcoind", "--regtest", "-txindex"]`
2. 利用 `docker build -t our-chain .` 創建 image

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

## 利用 gdb 執行配合智能搜尋

確保 container 內 vscode 安裝以下插件, 且無安裝衝突插件

- ms-vscode.cmake-tools
- ms-vscode.cpptools
- ms-vscode.cpptools-extension-pack
- ms-vscode.cpptools-themes
- twxs.cmake

可以使用 `code --list-extensions` 在 CLI 確認

此時配合 `.vscode` 內檔案應可以使用 debuger 和 智能提示

此外 `c_cpp_properties` 內可以視情況設置以下參數
範例為在 arm 晶片使用 clang 編譯配合 gnu17 標準

```
// 要用的編譯器標準
"cStandard": "gnu17",
"cppStandard": "gnu++17",
// 要用的編譯器
"intelliSenseMode": "clang-arm64"
```

## 參考用開發配置(bitcoin.conf)

若是需要客製化配置可以在 `.bitcoin` 下的設置檔自行修改配置，以下為預設配置

```
server=1
rpcuser=test
rpcpassword=test
rpcport=8332
rpcallowip=0.0.0.0/0
regtest=1
```

## 參考用發布腳本

利用 docker buildx 進行多平台發布，請確保使用有權限的 docker 帳號

```bash
git pull
docker buildx build --no-cache -t our-chain -f ./Dockerfile.prod --platform linux/amd64 .
docker tag our-chain your_docker_hub_id/our-chain
docker push your_docker_hub_id/our-chain
```

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
