#! /bin/sh

PWD=`pwd`

value=`cat ./flowgen_arg`

../../runtime/samples/dynamic_scale

sudo ../../deps/bess/bessctl/bessctl $value

sudo ../../deps/bess/bessctl/bessctl add connection fg rt1_iport_portout


