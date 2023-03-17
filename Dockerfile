FROM ubuntu:18.04
# main

RUN cd ~ && mkdir Desktop && cd Desktop && apt update && apt install git -y && git clone https://github.com/leon123858/OurChain.git && mv OurChain ourchain && cd ourchain && git checkout main
RUN apt-get install build-essential libtool autotools-dev automake pkg-config libssl-dev libevent-dev bsdmainutils libboost-all-dev software-properties-common wget net-tools -y && add-apt-repository ppa:bitcoin/bitcoin -y && apt-get update && apt-get install libdb4.8-dev libdb4.8++-dev -y

# (optional)
## miniupnpc
RUN apt-get install libminiupnpc-dev -y
## ZMQ
RUN apt-get install libzmq3-dev -y
## Qt 5
RUN apt-get install libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler -y
## libqrencode 
RUN apt-get install libqrencode-dev -y
## gmp
RUN apt-get install libgmp-dev libgmp3-dev -y 
## vim
RUN apt-get install vim -y

EXPOSE 22
EXPOSE 2500


# make
RUN cd ~/Desktop/ourchain && git pull && ~/Desktop/ourchain/autogen.sh && ~/Desktop/ourchain/configure --without-gui && make -j8 && make install && ldconfig && mkdir ~/.bitcoin/