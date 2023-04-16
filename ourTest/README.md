## OurChain
<details>
  <summary><span style="font-size: 1.5em; font-weight: bold;">目錄</span></summary>

- [安裝](#📥安裝)
- [修改](#✏️修改)
    - [新增RPC指令](#新增rpc指令)
    - [print 除錯](#print-除錯)
- [執行](#🚀執行)

</details>

---

## :inbox_tray:安裝

### 1. 開發環境設置
- 安裝 [Docker Desktop](https://www.docker.com/products/docker-desktop/)
- 安裝 [VSCode](https://code.visualstudio.com/)
- 安裝 [VSCode Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

### 2. 安裝必要檔案 (細節請見 [Dockerfile](/ourTest/zk/Dockerfile))
```bash
cd ourTest/zk
# 安裝 image 並命名為 ourchain
docker build -t ourchain .
# 從 ourchain 這個 image 生成 container，命名為 OurChain1，並啟動
docker run -it --name OurChain1 -p 18300-18320:18300-18320 ourchain
# 第二次開始只需使用下方指令就能重複使用 OurChain1
docker start OurChain1
```

### 3. 編譯 OurChain
```bash
# 以下皆在 container 內執行
cd ~/OurChain
# 切換到需要的分支 這裡以"zk"為例
git checkout zk
# build OurChain
./easymake.sh
```

## :pencil2:修改

### 新增RPC指令
1. 需要 rust 的話，適當的在 [/src/rustlib/src/lib.rs](/src/rustlib/src/lib.rs) 裡寫好程式
    - 需要的 header file 跟 library 可透過 `cargo build` 生成，但已經包含make裡了所以跳過
2. 適當的在 [/src/wallet/ourutil.cpp](/src/wallet/ourutil.cpp) 裡寫好程式
3. 適當的在 [/src/wallet/ourutil.h](/src/wallet/ourutil.h) 裡宣告程式
4. 適當的在 [/src/wallet/rpcwallet.cpp](/src/wallet/rpcwallet.cpp) 最下面插入程式
5. 適當的在 [/src/rpc/client.cpp](/src/rpc/client.cpp) 裡註記非字串輸入
6. `make`
7. 完成

### print 除錯
#### In *.cpp: 
```c
LogPrintf("debug message");
// log to /ourTest/<node_name>/regtest/debug.log
```

## :rocket:執行

### 使用 [test.sh](/test.sh)
```bash
# in folder OurChain
./test.sh <node_name> <rpc_command_or_special_command> [<rpc_arguments>...]
```
- node name: 即在 [node資料夾](/ourTest/node)內各資料夾名稱
- special command:
  - `start` 開啟節點
    - 請照 A ~ E 順序開啟，或是另外設定 addnode ，詳見各自的bitcoin.conf
  - `kill` 刪除節點資料
    - 請先 stop 後再 kill
  - `mine` 每兩秒自動挖礦
    - 前景執行，終止請用 `Ctrl + C`
- 範例:
```bash
./test.sh A start
./test.sh B start
./test.sh A generate 101
./test.sh B getnewaddress
# > mqYDWKz4eQvWNM3VJ2ws1ppZmu8L1M7uso
./test.sh A sendtoaddress mqYDWKz4eQvWNM3VJ2ws1ppZmu8L1M7uso 0.123
./test.sh A generate 1
./test.sh B getbalance
# > 0.123
./test.sh A stop
./test.sh B stop
./test.sh A kill
./test.sh B kill
```
