#! /bin/sh

# switch to the deps directory
cd deps

# start installing grpc
sudo apt-get install build-essential autoconf libtool
git clone -b $(curl -L http://grpc.io/release) https://github.com/grpc/grpc
cd grpc
git submodule update --init
make -j`nproc`
