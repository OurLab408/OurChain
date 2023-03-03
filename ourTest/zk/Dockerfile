#OS
FROM ubuntu:18.04

## dependencies
RUN apt-get update
RUN apt-get install \
  build-essential \
  libtool \
  autotools-dev \
  automake \
  pkg-config \
  libssl-dev \
  libevent-dev \
  bsdmainutils \
  software-properties-common \
  -y
RUN apt-add-repository ppa:bitcoin/bitcoin
RUN apt-get update
RUN apt-get install \
  libboost-all-dev \
  libdb4.8-dev \
  libdb4.8++-dev \
  python3 \
  git \
  curl \
  -y
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y --default-toolchain nightly

## clone ourchain & zokrates
WORKDIR /root/
RUN git clone https://github.com/ZoKrates/ZoKrates
RUN git clone https://github.com/OurLab408/OurChain

## build zokrates
WORKDIR /root/ZoKrates/
RUN /root/.cargo/bin/cargo +nightly build -p zokrates_cli --release
ENV ZOKRATES_HOME=/root/ZoKrates
ENV PATH "$ZOKRATES_HOME/target/release:$PATH"
ENV ZOKRATES_STDLIB "$ZOKRATES_HOME/zokrates_stdlib/stdlib"

## build ourchain
WORKDIR /root/OurChain/
RUN git checkout zk
## make
RUN ./autogen.sh
RUN ./configure --disable-gui --disable-tests
RUN make -j8
RUN make install
RUN ldconfig
