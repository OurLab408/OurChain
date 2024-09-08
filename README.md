# OurChain

## What is OurChain?

OurChain is a fork of Bitcoin Core that aims to provide a more efficient and autonomous blockchain. The main changes include, but are not limited to:

## Features

- GPoW (General Proof-of-Work) algorithm: a new PoW algorithm that is more efficient than the original PoW algorithm.
- OurContract: a new smart contract system that is based on the Bitcoin architecture.
- PT (PowerTimestamp): a global event ordering system that is based on OurChain.

## Quick Start

After starting the OurChain node, you can use the `bash mytest.sh` command to try basic contract rpc commands.

## Dev By Docker

Read [doc/dev-docker.md](doc/dev-docker.md).

## OurContract

Read [doc/ourcontractV2.md](doc/ourcontractV2.md).

## related projects

- [OurChain Agent](https://github.com/leon123858/ourchain-agent): A service that provides a REST API for OurChain. It supports mobile applications using RPC commands to interact with OurChain without running an OurChain node.
- [OurChain Admin Web](https://github.com/leon123858/ourChain-frontend/tree/main/ourchain-web-cli): A simple web application based on OurChain Agent that provides a user interface for OurChain. It allows users to deploy contracts, call contracts, and check contract status.
- [AID Type3 Demo](https://github.com/leon123858/aid/tree/main/demo/t3): An identity system that provide a demo implementation based on OurChain.

## License

The portions of this project related to Bitcoin are released under the terms of the [MIT License](https://opensource.org/licenses/MIT). The remaining parts are licensed under the [Benevolence License](https://hackmd.io/KpMx2d-wQd2t_gwQ97D9Cg#Benevolence-License).

Please note that the terms "OurChain", "OurCoin" (or "ourcoin" for a unit of OurCoin), "OurContract", "AID", and "PowerTimestamp" are used throughout this project to refer to specific implementations within it. These terms should not be confused with any other projects or terms that may share similar names.
