FROM ubuntu:18.04

# update package manager
RUN apt-get update -y

# install dev tools (only for development)
RUN apt-get install vim gdb -y
RUN apt-get install software-properties-common -y

# install git
RUN apt-get install git -y

# install core dependencies
RUN apt-get install build-essential libtool autotools-dev pkg-config bsdmainutils python3 -y
RUN apt-get install libevent-dev libboost-all-dev libssl-dev libdb++-dev -y
RUN apt-get install autoconf automake -y

#install rocksdb (constract db)
RUN apt-get install -y libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev
RUN cd ~ && git clone https://github.com/facebook/rocksdb.git && cd rocksdb && make shared_lib -j$(nproc --all) && make install-shared
RUN cd ~ && rm -rf rocksdb

# git clone ourchain
ARG REPO_URL=https://github.com/OurLab408/OurChain.git
ARG REPO_NAME=OurChain
ARG REPO_BRANCH=master
RUN cd ~ && mkdir Desktop && cd Desktop && git clone $REPO_URL && mv $REPO_NAME ourchain && cd ourchain && git checkout $REPO_BRANCH && git pull
WORKDIR /root/Desktop/ourchain

# install bitcoin optional dependencies
RUN apt-get install libzmq3-dev -y

# install ourchain dependencies
RUN apt-get install libgmp-dev -y


EXPOSE 22
EXPOSE 8332

# init
RUN ~/Desktop/ourchain/autogen.sh
RUN ~/Desktop/ourchain/configure --without-gui --with-incompatible-bdb --disable-tests --disable-bench

# set config
RUN mkdir ~/.bitcoin/
RUN echo -e "server=1\nrpcuser=test\nrpcpassword=test\nrpcport=8332\nrpcallowip=0.0.0.0/0\nregtest=1" >> /root/.bitcoin/bitcoin.conf

# compile
RUN make -j$(nproc --all) && make install && ldconfig

# run (only for production)
# ENTRYPOINT ["bitcoind", "--regtest", "-txindex"]
