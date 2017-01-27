#! /bin/sh

sudo kill -9 $(ps -ef | grep server_main | grep -v grep | awk '{print $2}')

sudo kill -9 $(ps -ef | grep dynamic_scale | grep -v grep | awk '{print $2}')

sudo rm ~/nfa/eval/dynamic_scale_test/*.log

touch state.log
sudo ../../deps/bess/bessctl/bessctl delete connection fg1 ogate


sudo ../../deps/bess/bessctl/bessctl delete connection fg2 ogate


sudo ../../deps/bess/bessctl/bessctl delete connection fg3 ogate


sudo ../../deps/bess/bessctl/bessctl delete connection fg4 ogate


sudo ../../deps/bess/bessctl/bessctl delete connection fg5 ogate


sudo ../../deps/bess/bessctl/bessctl delete connection fg6 ogate


