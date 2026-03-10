FROM ubuntu:22.04

# Prevent interactive prompts during package install
ENV DEBIAN_FRONTEND=noninteractive

# Update package manager and install dev tools
RUN apt-get update -y \
    && apt-get install -y --no-install-recommends \
        vim gdb \
        software-properties-common \
        git \
        build-essential libtool autotools-dev pkg-config bsdmainutils python3 \
        libevent-dev libboost-all-dev libssl-dev libdb++-dev \
        autoconf automake \
        libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Build and install RocksDB (use explicit path; each RUN starts fresh)
RUN cd /root \
    && git clone --branch v10.4.2 https://github.com/facebook/rocksdb.git \
    && cd rocksdb \
    && make shared_lib -j$(nproc) \
    && make install-shared \
    && cd /root \
    && rm -rf rocksdb

# Copy current OurChain folder into image (build context = project root)
RUN mkdir -p /root/Desktop/ourchain
COPY . /root/Desktop/ourchain

WORKDIR /root/Desktop/ourchain

# Install remaining dependencies (second apt layer for ourchain-specific deps)
RUN apt-get update -y \
    && apt-get install -y --no-install-recommends libzmq3-dev libgmp-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

EXPOSE 22
EXPOSE 8332

# Bootstrap and configure (C++ standard from configure.ac: AX_CXX_COMPILE_STDCXX 17)
RUN ./autogen.sh \
    && ./configure --without-gui --with-incompatible-bdb --disable-tests --disable-bench

# Set config
RUN mkdir -p /root/.bitcoin \
    && echo -e "server=1\nrpcuser=test\nrpcpassword=test\nrpcport=8332\nrpcallowip=0.0.0.0/0\nregtest=1" >> /root/.bitcoin/bitcoin.conf

# Compile
RUN make -j$(nproc) \
    && make install \
    && ldconfig

# Run (only for production)
# ENTRYPOINT ["bitcoind", "--regtest", "-txindex"]
