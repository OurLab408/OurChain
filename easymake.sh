./autogen.sh
./configure --disable-gui --disable-tests
make -j8
make install
ldconfig
