#! /bin/sh

PWD=`pwd`

value=`cat ./flowgen_arg2`


sudo ../../deps/bess/bessctl/bessctl $value

sudo ../../deps/bess/bessctl/bessctl delete module fg
sudo ../../deps/bess/bessctl/bessctl add connection fg2 rt1_iport_portout

