#! /bin/sh

# record current working directory
PWD=`pwd`

# the current working branch of bess
BESS_BRANCH="c++"

# switch to the deps directory
cd deps

# install grpc and protocbuf
if [ ! -d "./grpc" ]; then

  # build and install grpc
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
else
  echo "grpc and protobuf have already been installed."
fi

# switch to deps directory
cd $PWD/deps

# install several packages that are required to build bess
sudo apt-get install libssl-dev libunwind8-dev liblzma-dev

# download and build bess
if [ ! -d "./bess" ]; then
  git clone -b $BESS_BRANCH https://github.com/NetSys/bess.git
  cd bess
  ./build.py
else
  echo "bess has already been built."
fi
