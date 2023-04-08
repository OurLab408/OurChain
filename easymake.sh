cargo build --manifest-path=src/rustlib/Cargo.toml
./autogen.sh
./configure --disable-gui --disable-tests
make -j8
make install
ldconfig
