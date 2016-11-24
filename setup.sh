#! /bin/sh

# record current working directory
PWD=`pwd`

# switch to the deps directory
cd deps

# start installing grpc
sudo apt-get install build-essential autoconf libtool
git clone -b $(curl -L http://grpc.io/release) https://github.com/grpc/grpc
cd grpc
git submodule update --init
make -j`nproc`
sudo make install

# go to /third_party/protobuf and install protobuf
cd ./third_party/protobuf
make -j`nproc`
sudo make install

# switch to nfa directory
cd $PWD
