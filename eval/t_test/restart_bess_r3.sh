#! /bin/sh

sudo ~/nfa/deps/bess/bessctl/bessctl daemon reset

sudo ~/nfa/deps/bess/bessctl/bessctl run file ~/nfa/eval/t_test/bess_r3_script
