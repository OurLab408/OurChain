#!/bin/bash

if [ -z "$1" ]; then
    echo "need to set which bitcoin directory you want to install" 
    exit
fi

cd $1
echo "choose mode 'reconfig' or 'install' or 'clean' or not"
read mode

if [[ $mode == "reconfig" ]]; then
    cd src/
    echo m9031314 | sudo -S rm -rf *.a *.o *.la *.lo .libs/ .deps/ */*.a */*.o */*.la */*.lo */.deps */.libs */.dirstamp
    cd ..
    ./autogen.sh
    ./configure --disable-gui --disable-tests
    make -j4
elif [[ $mode == "install" ]]; then
    echo m9031314 | sudo -S make -j4 install
    echo m9031314 | sudo -S ldconfig
elif [[ $mode == "clean" ]]; then
    cd src/
    echo m9031314 | sudo -S rm -rf *.a *.o *.la *.lo .libs/ .deps/ */*.a */*.o */*.la */*.lo */.deps */.libs */.dirstamp
    rm -rf ~/.bitcoin/regtest
else
    make -j4
fi

