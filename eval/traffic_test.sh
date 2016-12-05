#! /bin/sh

value=`cat ./flowgen_arg`

sudo ../deps/bess/bessctl/bessctl $value
