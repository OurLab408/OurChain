FROM ubuntu:18.04
# main

RUN cd ~ && mkdir Desktop && cd Desktop && apt update && apt install git -y && git clone https://bitbucket.org/lab408/ourchain-release.git && mv ourchain-release test && cd test && git checkout tps_finality
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
## ssh
RUN apt-get install ssh -y 
## sshpass
RUN apt-get install sshpass -y
RUN sed -i 's/#   StrictHostKeyChecking ask/   StrictHostKeyChecking no/' /etc/ssh/ssh_config

##ssh login
RUN apt-get install -y openssh-server
RUN mkdir /var/run/sshd
RUN echo 'root:asdf' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -i 's/root:x/lab408:x/' /etc/passwd
RUN sed -i 's/root:/lab408:/' /etc/shadow

# SSH login fix. Otherwise user is kicked off after login
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]


# make
RUN cd ~/Desktop/test && ~/Desktop/test/autogen.sh && ~/Desktop/test/configure --without-gui --disable-tests && make -j8 && make install && ldconfig && mkdir ~/.bitcoin/
RUN cd ~/Desktop/test/testtps && make && ./docker_clean.sh

