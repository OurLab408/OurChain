## 如何使用 [test.sh](/test.sh)
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

## 如何新增RPC指令
1. 需要 rust 的話，適當的在 [/src/rustlib/src/lib.rs](/src/rustlib/src/lib.rs) 裡寫好程式
    - 需要的 header file 跟 library 可透過 `cargo build` 生成，但已經包含make裡了所以跳過
2. 適當的在 [/src/wallet/ourutil.cpp](/src/wallet/ourutil.cpp) 裡寫好程式
3. 適當的在 [/src/wallet/ourutil.h](/src/wallet/ourutil.h) 裡宣告程式
4. 適當的在 [/src/wallet/rpcwallet.cpp](/src/wallet/rpcwallet.cpp) 最下面插入程式
5. 適當的在 [/src/rpc/client.cpp](/src/rpc/client.cpp) 裡註記非字串輸入
6. `make`
7. 完成

## print / console.log 大法
### In *.c / *.cpp: 
```c
LogPrintf("debug message");
// log to /ourTest/<node_name>/regtest/debug.log
```