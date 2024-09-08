# dev by docker

## environment

- ref: https://www.docker.com/ install docker
- ref: https://www.docker.com/products/docker-desktop/ install docker desktop
- ref: https://code.visualstudio.com/ install vscode
- ref: https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers install vscode container plugin

## first time setup

in project root

```bash
# install image (don't want cache => `docker build --no-cache -t our-chain .`)
docker build -t our-chain .
# generate container from image and run
docker run --name our-chain -it our-chain
# when you want to use OurChain rpc outside container, you need to expose port 8332
docker run --name our-chain -it -p 8332:8332 our-chain
```

use below command in container, you can interact with OurChain Node

```bash
# Start OurChain Node
./src/bitcoind --regtest --daemon -txindex
# Stop OurChain Node
./src/bitcoin-cli stop
# Get help
./src/bitcoin-cli help
# Get embedded address's balance
./src/bitcoin-cli getbalance
# mining 1 block for myself
./src/bitcoin-cli generate 1
# deploy contract (can check info in ~/.bitcoin/regtest/contracts)
./src/bitcoin-cli deploycontract ~/Desktop/ourchain/sample.cpp
# call contract
./src/bitcoin-cli callcontract "contract address when deploy" "arg1" "arg2" ...
# get contract message, this command is "pure", it will not change the state of the contract
./src/bitcoin-cli dumpcontractmessage "contract address" ""
```

can use `bash ./mytest.sh` run contract commands

important path in container

- in `/root/Desktop/ourchain` you can find the project
- in `/root/.bitcoin/bitcoin.conf` you can find the bitcoin config
- in `/root/.bitcoin/regtest/contracts` you can find the OurContract info

## restart container

```bash
# list container ID
docker container ls -a
# restart container
docker start [CONTAINER ID]
```

## deploy container

1. add command `ENTRYPOINT ["bitcoind", "--regtest", "-txindex"]` in dockerfile
2. use `docker build -t our-chain .` create image

## file edit and execute

### edit

- Open VS Code
- In the leftmost row of icons, click Remote Explorer
- After pressing the correct Container, press Attach to Container in the icon that appears
- Then a new VS Code will be automatically opened, and the green part in the lower left corner will display the Image ID of the Container.
- In the leftmost row of icons, click Explorer
- Select the path where ourchain is located and open it

### execute

- Select Terminal > New Terminal from the top toolbar of VS Code or use the shortcut key [Ctrl+Shift+`] to open the Terminal
- You can also use docker desktop to open the Terminal inside the container

## setup gdb and smart hint in vscode

Make sure that vscode in the container has the following plug-ins installed, and no conflicting plug-ins are installed.

- ms-vscode.cmake-tools
- ms-vscode.cpptools
- ms-vscode.cpptools-extension-pack
- ms-vscode.cpptools-themes
- twxs.cmake

use `code --list-extensions` can check installed extensions

At this time, you should be able to use debugger and smart prompts with the files in `.vscode`

In addition, the following parameters can be set in `c_cpp_properties` as appropriate.

The example is using clang to compile with the gnu17 standard on an arm chip.

```
// compiler spec
"cStandard": "gnu17",
"cppStandard": "gnu++17",
// compiler name
"intelliSenseMode": "clang-arm64"
```

## bitcoin core coding style

use below command to format code

```
git diff -U0 HEAD~1.. | ./contrib/devtools/clang-format-diff.py -p1 -i -v
```

you can format 1 commit before HEAD.

## sample bitcoin.conf

If you need customized configuration, you can modify the configuration yourself in the configuration file under `.bitcoin`. The following is the default configuration.

```
server=1
rpcuser=test
rpcpassword=test
rpcport=8332
rpcallowip=0.0.0.0/0
regtest=1
```

## Reference release script

To use docker buildx for multi-platform publishing, please make sure to use a docker account with permissions

```bash
git pull
docker buildx build --no-cache -t our-chain -f ./Dockerfile.prod --platform linux/amd64 .
docker tag our-chain your_docker_hub_id/our-chain
docker push your_docker_hub_id/our-chain
```

## delete container

use GUI in docker desktop
