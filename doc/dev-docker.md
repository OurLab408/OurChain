# OurChain Development with Docker

## Prerequisites

Before starting, ensure you have the following installed:

- [Docker](https://www.docker.com/) - Container platform
- [Docker Desktop](https://www.docker.com/products/docker-desktop/) - Docker GUI (recommended)
- [VS Code](https://code.visualstudio.com/) - Code editor
- [Remote Containers Extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) - VS Code extension for container development

## First Time Setup

Navigate to the project root directory and run:

```bash
# Build the development image (use --no-cache for clean build)
docker build -t our-chain .

# Run development container (interactive mode)
docker run --name our-chain -it our-chain

# For RPC access from outside container, expose port 8332
docker run --name our-chain -it -p 8332:8332 our-chain
```

## Basic OurChain Commands

Once inside the container, you can interact with the OurChain node:

```bash
# Start OurChain Node
./src/bitcoind --regtest --daemon -txindex

# Stop OurChain Node
./src/bitcoin-cli stop

# Get help
./src/bitcoin-cli help

# Get embedded address's balance
./src/bitcoin-cli getbalance

# Mining 1 block for myself
./src/bitcoin-cli generate 1

# Deploy contract (check info in ~/.bitcoin/regtest/contracts)
./src/bitcoin-cli deploycontract ./src/test/test_contract.cpp

# Call contract
./src/bitcoin-cli callcontract "contract_address" "arg1" "arg2" ...

# Get contract message (read-only, doesn't change state)
./src/bitcoin-cli dumpcontractmessage "contract_address" ""
```

**Quick Test**: Use `bash ./mytest.sh` to run the complete contract test suite.

## Important Paths in Container

- **Project directory**: `/root/ourchain` - Main OurChain source code
- **Bitcoin config**: `/root/.bitcoin/bitcoin.conf` - Node configuration
- **Contract data**: `/root/.bitcoin/regtest/contracts` - Deployed contract information

## Container Management

```bash
# List all containers
docker container ls -a

# Restart existing container
docker start our-chain

# Attach to running container
docker exec -it our-chain /bin/bash

# Stop container
docker stop our-chain
```

## Production Deployment

For production deployment, use the optimized production Dockerfile:

```bash
# Build production image
docker build -f Dockerfile.prod -t our-chain-prod .

# Run production container (auto-starts bitcoind)
docker run -d -p 8332:8332 --name our-chain-prod our-chain-prod
```

## VS Code Development Setup

### Connect to Container

1. Open VS Code
2. Click **Remote Explorer** in the left sidebar
3. Find your container and click **Attach to Container**
4. VS Code will open a new window connected to the container
5. Open the project folder: `/root/ourchain`

### Terminal Access

- **VS Code Terminal**: `Ctrl+Shift+`` (backtick) or Terminal > New Terminal
- **Docker Desktop**: Use the terminal feature in Docker Desktop GUI
- **Command Line**: `docker exec -it our-chain /bin/bash`

## VS Code Extensions & Debugging

### Required Extensions

Install these VS Code extensions for optimal C++ development:

- `ms-vscode.cmake-tools` - CMake support
- `ms-vscode.cpptools` - C/C++ IntelliSense
- `ms-vscode.cpptools-extension-pack` - Complete C++ toolkit
- `ms-vscode.cpptools-themes` - Syntax highlighting
- `twxs.cmake` - CMake language support

**Check installed extensions**: `code --list-extensions`

### C++ Configuration

For proper IntelliSense and debugging, configure `c_cpp_properties.json`:

```json
{
    "configurations": [
        {
            "name": "Linux",
            "cStandard": "gnu20",
            "cppStandard": "gnu++20",
            "intelliSenseMode": "gcc-arm64",
            "compilerPath": "/usr/bin/gcc"
        }
    ]
}
```

**Note**: Updated to C++20 standard to match the current build configuration.

## Code Formatting

Format your code according to Bitcoin Core standards:

```bash
# Format the last commit
git diff -U0 HEAD~1.. | ./contrib/devtools/clang-format-diff.py -p1 -i -v

# Format specific files
./contrib/devtools/clang-format-diff.py -p1 -i -v < file.diff
```

## Configuration

### Default bitcoin.conf

The container comes with a pre-configured `bitcoin.conf`:

```ini
server=1
rpcuser=test
rpcpassword=test
rpcport=8332
rpcallowip=0.0.0.0/0
regtest=1
```

**Location**: `/root/.bitcoin/bitcoin.conf`

### Custom Configuration

To modify settings, edit the configuration file directly or create a new one with your preferred settings.

## Multi-Platform Release

For publishing to Docker Hub with multi-platform support:

```bash
# Pull latest changes
git pull

# Build for multiple platforms
docker buildx build --no-cache -t our-chain -f ./Dockerfile.prod --platform linux/amd64,linux/arm64 .

# Tag and push to Docker Hub
docker tag our-chain your_docker_hub_id/our-chain
docker push your_docker_hub_id/our-chain
```

**Note**: Requires Docker Hub account with appropriate permissions.

## Cleanup

### Remove Containers

```bash
# Stop and remove container
docker stop our-chain
docker rm our-chain

# Remove image
docker rmi our-chain
```

**Alternative**: Use Docker Desktop GUI for container management.
