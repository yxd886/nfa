#! /bin/sh

PWD=`pwd`

value1=`cat ./flowgen_arg`

value2=`cat ./flowgen_arg_noduplicate`

../../runtime/samples/nfa_rpc_client

sudo ../../deps/bess/bessctl/bessctl $value1

sudo ../../deps/bess/bessctl/bessctl $value2

sudo ../../deps/bess/bessctl/bessctl add connection fg rt1_iport_portout

sudo ../../deps/bess/bessctl/bessctl add connection fg1 rt1_iport_portout

