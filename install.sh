#!/bin/bash

mkdir build
cd build
sudo ../autogen.sh
sudo ../configure --without-gui --disable-tests
sudo make -j`nproc`
sudo make install 
sudo ldconfig
