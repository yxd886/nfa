#! /bin/sh

PWD=`pwd`

value=`cat ./flowgen_arg`

sudo ../../deps/bess/bessctl/bessctl $value

sudo ../../deps/bess/bessctl/bessctl add connection fg rt1_iport_portout

../../runtime/samples/nfa_rpc_client
